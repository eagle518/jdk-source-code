/*
 * @(#)propertyParser.c	1.31 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "util.h"
#include "propertyParser.h"
#include "configurationFile.h"

/* Local methods definitions */
static char* GetNextOption(char* s, char** option, char **value, char **read);

int GetJREIndex(char *key) {
  char *keydup = strdup(key);
  unsigned int subkeylen = strlen(CFG_JRE_KEY);
  int index = -1;
  
  if (strlen(keydup) >= subkeylen && 
      strncmp(keydup, CFG_JRE_KEY, subkeylen)==0) {
    char *p = &(keydup[subkeylen]);
    if (p) {
      char *q = strchr(p, '.');
      if (q) {
        *q = 0;
        index = atoi(p);
      }
    }
  }
  free(keydup);
  return index;
}



/*
 * At this point the property list looks like this :
 *
 *       entry <-- userHead
 *         |
 *         V
 *       entry
 *         |
 *         V
 *     (more entries)
 *         |
 *         V
 *        NULL
 *
 * We want to store everything between userHead and installHead (i.e. user
 * specific properties) in the user properties file.
 */
void storePropertyFile(char *filename, PropertyFileEntry* userHead) {
  PropertyFileEntry *entry;
  FILE *fp = NULL;
  int index;
  char *dir, *p;

  /* create directories if they don't exist */
  dir = strdup(filename);
  p = strrchr(dir, FILE_SEPARATOR);
  if (p != NULL) {
    if (p > dir && *(p-1) != ':') *p = 0;
    else *(p+1) = 0;
    recursive_create_directory(dir);
  }
  free(dir);

  fp = fopen(filename, "wb");
  if (!fp) return;

  for (entry = userHead; entry != NULL; entry = entry->next) {
    if ((index = GetJREIndex(entry->key)) == -1 ||
        isJRERegistered(index) == 0 ||
        isJREConfirmed(index) == 1 ) {

      /* if it's a regular (non JRE) entry, or an unregistered JRE, or a
         confirmed JRE, write it to the file */
      if (entry->key != NULL && entry->value != NULL) {
        char *buffer = calloc(2, strlen(entry->key) + strlen(entry->value) + 8);

        int i = 0;
        char *p = entry->key;
        while (*p != 0) {
          if (*p == '\\') {
            buffer[i++] = '\\';
          }
          buffer[i++] = *p++;
        }
        buffer[i++] = '=';
        if (entry->read_value != NULL) {
          strcat(buffer, entry->read_value);
          strcat(buffer, "\n");
          fwrite(buffer, 1, strlen(buffer), fp); 
        } else {
          char *output = NULL;
          p = entry->value;
          while (*p != 0) {
            /* perhaps have unicode in file */
            if (*p == '\\') {
              /* ok - either already escaped, or need to escape */
              if (*(p+1) == '\\') {
                /* already escaped, put two slashes in as they were */
                buffer[i++] = *p++;
                buffer[i++] = *p++;
              } else if (*(p+1) == ':') {
                /* already escape - colon - so leave as is. */
                buffer[i++] = *p++;
                buffer[i++] = *p++;
              } else {
                /* neither, so need to escape the backslash */
                buffer[i++] = '\\';
                buffer[i++] = *p++;
              }
            } else {
              buffer[i++] = *p++;
            }
          }
          buffer[i] = 0;
          strcat(buffer, "\n");
          output = sysMBCSToSeqUnicode(buffer); 
          fwrite(output, 1, strlen(output), fp); 
          free(output); 
        }
        free(buffer);
      }
    }
  }
  fclose(fp);
}


/*
 * Parses a property file. Returns NULL if file not found or it is empty 
 */
PropertyFileEntry* parsePropertyFile(char *filename, PropertyFileEntry* head) {
    PropertyFileEntry* entry = NULL;
    char* buffer;
    int size;

    /* Read contents of file into memory */
    size  = ReadFileToBuffer(filename, &buffer);
    /* File not found? */
    if (buffer == NULL) return head;
    
    entry = parsePropertyStream(buffer, head);

    free(buffer);
    return entry;        
}

/*
 * Parses a string buffer as a property file
 */
PropertyFileEntry* parsePropertyStream(char *s, PropertyFileEntry* head) {
    PropertyFileEntry* entry = NULL;
    char* key;
    char* value;
    char* read_value;

    s = GetNextOption(s, &key, &value, &read_value);
    while(s != NULL) {
        /* Construct new entry element */
        PropertyFileEntry* entry = (PropertyFileEntry*)malloc(sizeof(PropertyFileEntry));
        entry->key   = key;
        entry->value = value;
        entry->read_value = read_value;
        /* record JRE index if this is a JRE */
        if (key != NULL) {
          int index;
          if ((index = GetJREIndex(key)) != -1) {
            addToIndexArray(index);
          }
        }   
        entry->next  = head;
        head = entry;
        /* Parse next entry */
        s = GetNextOption(s, &key, &value, &read_value);
    }        
    return head;
}

/* This interates through a property file, returning one
 * (option,value) pair at a time. It automatically skips
 * comments and blank lines. Returns true if a pair is
 * found, false when at the end of the file.
 *
 * The file is parsed pretty much according to the spec.
 * for the java.util.Properties output format. However,
 * multiline values and unicode escaped values are not
 * supported. (This should be removed/converted in a pre-parse)
 */
static char* GetNextOption(char* p, char** optionPtr, 
                 char **valuePtr, char **readValuePtr) {
    char* mark;
    char* str;
    char* end;
    int len;
    
    *optionPtr = NULL;
    *valuePtr  = NULL;
    *readValuePtr = NULL;

    /* Check if we are at the end */
    if (p == NULL || *p == '\0') return NULL;

    /* Skip whitespace, newlines, and comments */
    do {
        mark = p;
        while(iswspace(*p) || *p == '\r' || *p == '\n') p++;
        if (*p == '#') {
            p++;
            while(*p && (*p != '\n' && *p != '\r')) p++;
        }
    } while(mark != p);    

    /* Are at end of the buffer? */
    if (!(*p)) return NULL;

    /* Parse key */
    mark = p;
    
    /* Find end of option */
    while(*p && (!(iswspace(*p) || *p == ':' || *p == '=')) || 
                 ((mark != p) && (*(p-1) == '\\'))) p++;

    /* Allocate memory for key */
    len = p - mark;

    str = (char*)malloc(len + 1);
    strncpy(str, mark, len);
    str[len] = '\0';
    *optionPtr = str;

    /* Skip until start of value */
    while(iswspace(*p)) p++;
    if (*p && (*p == ':' || *p == '=')) {
        p++;
        while(iswspace(*p) && *p != '\n' && *p != '\r') p++;
    }
    /* End of stream? */
    if (!(*p)) return NULL;

    /* Find end of value and '\' is a joining of sentence  */
    mark = p;
    while(*p && (*p != '\n'|| *(p-1) == '\\') 
                        && (*p != '\r'|| *(p-1) == '\\')) p++;

    /* Trim trailing whitespaces */
    end = p;
    while(end > mark && iswspace(end[-1])) end--;

    /** Allocate buffer for value */
    len = end - mark;
    str = (char*)malloc(len + 1);
    strncpy(str, mark, len);
    str[len] = '\0';
    *valuePtr = str;

    /* retain the original unmodified value */
    *readValuePtr = strdup(str);
    
    /* Handle potential escape characters in value */
    mark = str;
    while (*str) {
        if (*str != '\\' || str[1] == 'u') {
            *mark = *str;
            mark++; str++;
        } else {
            switch(*(++str)) {
                case 't': *mark = '\t'; break;
                case 'n': *mark = '\n'; break;
                case 'r': *mark = '\r'; break;
                default : *mark = *str;
            }
            if (*str) str++;                    
            mark++;
        }
    }
    *mark = '\0';

    return p;
}

/* Find a specific entry */
char* GetPropertyValue(PropertyFileEntry* head, char* key) {
    char *match = NULL;

    /* search through the entire list because later entries should override */
    /* earlier entries */
    while(head != NULL) {
        if (strcmp(head->key, key) == 0) {
            match = head->value;
        }
        head = head->next;
    }        
    return match;
}

PropertyFileEntry *AddProperty(PropertyFileEntry *head, char *key, char *value) {

    PropertyFileEntry *entry = head;
    PropertyFileEntry *prev = NULL;

    while(entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            /* ok need to remove this one */
            if (prev == NULL) {
                head = entry->next;
            } else {
                prev->next = entry->next;
            }
               free(entry->key);
                   free(entry->value);
        } else {
            prev = entry;
        }
        entry = entry->next;
    }
    if (value != NULL) {
        /* ok add new one */
        entry = (PropertyFileEntry*)malloc(sizeof(PropertyFileEntry));
        entry->key = strdup(key);
        entry->value = strdup(value);
        entry->read_value = NULL;
        entry->next = head;
        return entry;
    }
    return head;
}


/* Release all memory used by the property entry */
void FreePropertyEntry(PropertyFileEntry* head) {
    PropertyFileEntry* next = NULL;

    while(head != NULL) {
        next = head->next;
        if (head->key != NULL) {
            free(head->key);
        }
        if (head->value != NULL) {
            free(head->value);
        }
        if (head->read_value != NULL) {
            free (head->read_value);
        }
        free(head);
        head = next;        
    }        
}

void PrintSinglePropertyEntry(PropertyFileEntry* entry) {
    if (entry == NULL) {
        printf("---NULL---\n");
    } else {
        printf("PROP (%s, %s)\n", entry->key, entry->value);
    }
}

void PrintPropertyEntry(PropertyFileEntry* entry) {
    if (entry == NULL) {
        printf("---end---\n");
    } else {
        printf("PROP (%s, %s)\n", entry->key, entry->value);
        PrintPropertyEntry(entry->next);
    }
}
