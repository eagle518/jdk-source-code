/*
 * @(#)xmlparser.c	1.19 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "system.h"
#include "util.h"
#include "xmlparser.h"
#include "msgString.h"

#include <sys/stat.h>
#include <ctype.h>

/* Internal declarations */
static XMLNode*      ParseXMLElement(void);
static XMLAttribute* ParseXMLAttribute(void);
static char*         SkipWhiteSpace(char *p);
static char*         SkipXMLName(char *p);
static char*         SkipXMLComment(char *p);
static char*         SkipXMLDocType(char *p);
static char*         SkipXMLProlog(char *p);
static char*         SkipPCData(char *p);
static int           IsPCData(char *p);
static void          ConvertBuiltInEntities(char* p);
static void          SetToken(int type, char* start, char* end);
static void          GetNextToken(void);
static XMLNode*      CreateXMLNode(int type, char* name);
static XMLAttribute* CreateXMLAttribute(char *name, char* value);
static XMLNode*      ParseXMLElement(void);
static XMLAttribute* ParseXMLAttribute(void);
static void          FreeXMLAttribute(XMLAttribute* attr);
static void          PrintXMLAttributes(XMLAttribute* attr);
static void          indent(int indt);

/** Iterates through the null-terminated buffer (i.e., C string) and replaces all
 *  UTF-8 encoded character >255 with 255
 *
 *  UTF-8 encoding:
 *
 *   Range A:  0x0000 - 0x007F 
 *                               0 | bits 0 - 7
 *   Range B : 0x0080 - 0x07FF  : 
 *                               110 | bits 6 - 10
 *                               10  | bits 0 - 5
 *   Range C : 0x0800 - 0xFFFF  :        
 *                               1110 | bits 12-15
 *                               10   | bits  6-11
 *                               10   | bits  0-5
 */
static void RemoveNonAsciiUTF8FromBuffer(char *buf) {
    char* p;
    char* q;
    char c;
    p = q = buf;
    while(*p != '\0') {
        c = *p;
        if ( (c & 0x80) == 0) {
            /* Range A */
            *q++ = *p++;            
        } else if ((c & 0xE0) == 0xC0){
            /* Range B */
            *q++ = (char)0xFF;
            p++;
            if (*p) p++;
        } else {
            /* Range C */
            *q++ = (char)0xFF;
            p++;
            if (*p) p++;
            if (*p) p++;            
        }        
    }
    /* Null terminate string */
    *q = '\0';    
}

/* --------------------------------------------------------------------- */

static char* SkipWhiteSpace(char *p) {
    if (p != NULL) {       
        while(*p == 0x20 || *p == 0x09 || *p == 0x0d || *p == 0x0A) p++;
    }
    return p;
}

static char* SkipXMLName(char *p) {
    char c = *p;
    /* Check if start of token */
    if ( ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') || 
         c == '_' || c == ':') {
        
        while( ('a' <= c && c <= 'z') ||
               ('A' <= c && c <= 'Z') || 
               ('0' <= c && c <= '9') ||
               c == '_' || c == ':' || c == '.' || c == '-' ) {
            p++;
            c = *p;
	    if (c == '\0') break;
        }        
    }
    return p;
}

static char* SkipXMLComment(char *p) {
    if (p != NULL) {
        if (strncmp(p, "<!--", 4) == 0) {
            p += 4;
            do {
                if (strncmp(p, "-->", 3) == 0) return p+3;
                p++;
            } while(*p != '\0');         
        }
    }
    return p;
}

static char* SkipXMLDocType(char *p) {
    if (p != NULL) {
        if (strncmp(p, "<!", 2) == 0) {
            p += 2;
            while (*p != '\0') {
                if (*p == '>') {
                    return p+1;
                }
                p++;
            }
        }
    } 
    return p;
}

static char* SkipXMLProlog(char *p) {
    if (p != NULL) {
        if (strncmp(p, "<?", 2) == 0) {
            p += 2;
            do {
                if (strncmp(p, "?>", 2) == 0) return p+2;
                p++;
            } while(*p != '\0');            
        }
    }
    return p;
}

/* Search for the built-in XML entities:
 * &amp; (&), &lt; (<), &gt; (>), &apos; ('), and &quote(")
 * and convert them to a real character
 */
static void ConvertBuiltInEntities(char* p) {
    char* q;
    q = p;
    while(*p) {
      if (IsPCData(p)) {
	/* dont convert &xxx values within PData */
        char *end;
        end = SkipPCData(p);
        while(p < end) {
	    *q++ = *p++;
	}
      } else {
        if (strncmp(p, "&amp;", 5) == 0) {
            *q = '&';
            q++;
            p += 5;
        } else if (strncmp(p, "&lt;", 4)  == 0) {
            *q = '<';
            p += 4;
        } else if (strncmp(p, "&gt;", 4)  == 0) {
            *q = '>';
            p += 4;
        } else if (strncmp(p, "&apos;", 6)  == 0) {
            *q = '\'';
            p += 6;
        } else if (strncmp(p, "&quote;", 7)  == 0) {
            *q = '\"';
            p += 7;
        } else {
            *q++ = *p++;
        }
      }
    }
    *q = '\0';
}

/* ------------------------------------------------------------- */
/* XML tokenizer */

#define TOKEN_UNKNOWN             0
#define TOKEN_BEGIN_TAG           1  /* <tag */
#define TOKEN_END_TAG             2  /* </tag */
#define TOKEN_CLOSE_BRACKET       3  /* >  */
#define TOKEN_EMPTY_CLOSE_BRACKET 4  /* /> */
#define TOKEN_PCDATA              5  /* pcdata */
#define TOKEN_CDATA               6  /* cdata */
#define TOKEN_EOF                 7

static char* CurPos       = NULL;
static char* CurTokenName        = NULL;
static int   CurTokenType;
static int   MaxTokenSize = -1;

/* Copy token from buffer to Token variable */
static void SetToken(int type, char* start, char* end) {
    int len = end - start;
    if (len > MaxTokenSize) {
        if (CurTokenName != NULL) free(CurTokenName);
        CurTokenName = (char *)malloc(len + 1);
        MaxTokenSize = len;
    }
    
    CurTokenType = type;
    strncpy(CurTokenName, start, len);
    CurTokenName[len] = '\0';    
}

/* Skip XML comments, doctypes, and prolog tags */
static char* SkipFilling(void) {
    char *q = CurPos;

    /* Skip white space and comment sections */
    do {
        q = CurPos;        
        CurPos = SkipWhiteSpace(CurPos);
        CurPos = SkipXMLComment(CurPos); /* Must be called befor DocTypes */
        CurPos = SkipXMLDocType(CurPos); /* <! ... > directives */
        CurPos = SkipXMLProlog(CurPos);   /* <? ... ?> directives */
    } while(CurPos != q);

    return CurPos;
}


/* Parses next token and initializes the global token variables above 
   The tokennizer automatically skips comments (<!-- comment -->) and
   <! ... > directives.
*/
static void GetNextToken(void) {
    char *p, *q;

    /* Skip white space and comment sections */
    p = SkipFilling();

    if (p == NULL || *p == '\0') {
        CurTokenType = TOKEN_EOF;
        return;
    } else if (p[0] == '<' && p[1] == '/') {
        /* TOKEN_END_TAG */
        q = SkipXMLName(p + 2);        
        SetToken(TOKEN_END_TAG, p + 2, q);        
        p = q;
    } else  if (*p == '<') {
        /* TOKEN_BEGIN_TAG */
        q = SkipXMLName(p + 1);        
        SetToken(TOKEN_BEGIN_TAG, p + 1, q);        
        p = q;
    } else if (p[0] == '>') {
        CurTokenType = TOKEN_CLOSE_BRACKET;                
        p++;
    } else if (p[0] == '/' && p[1] == '>') {
        CurTokenType = TOKEN_EMPTY_CLOSE_BRACKET;        
        p += 2;
    } else {                
        /* Search for end of data */
        q = p + 1;
        while(*q && *q != '<') {
	    if (IsPCData(q)) {
		q = SkipPCData(q);
	    } else {
		q++;
	    }
	}
        SetToken(TOKEN_PCDATA, p, q);
        /* Convert all entities inside token */
        ConvertBuiltInEntities(CurTokenName);
        p = q;
    }
    /* Advance pointer to beginning of next token */
    CurPos = p;
}


static XMLNode* CreateXMLNode(int type, char* name) {
    XMLNode* node;
    node  = (XMLNode*)malloc(sizeof(XMLNode));
    node->_type = type;
    node->_name = name;
    node->_next = NULL;
    node->_sub  = NULL;
    node->_attributes = NULL;
    return node;
}


static XMLAttribute* CreateXMLAttribute(char *name, char* value) {
    XMLAttribute* attr;
    attr = (XMLAttribute*)malloc(sizeof(XMLAttribute));
    attr->_name = name;
    attr->_value = value;
    attr->_next =  NULL;
    return attr;
}


XMLNode* ParseXMLDocument(char* buf) {
    XMLNode* root;

    /* Remove UTF-8 encoding from buffer */
    RemoveNonAsciiUTF8FromBuffer(buf);

    /* Get first Token */
    CurPos = buf;
    GetNextToken();
    
    
    /* Parse document*/
    root =  ParseXMLElement();

    return root;
}

static XMLNode* ParseXMLElement(void) {
    XMLNode*  node     = NULL;
    XMLNode*  subnode  = NULL;
    XMLNode*  nextnode = NULL;
    XMLAttribute* attr = NULL;
    
    if (CurTokenType == TOKEN_BEGIN_TAG) { 

        /* Create node for new element tag */
        node = CreateXMLNode(xmlTagType, strdup(CurTokenName));
    
        /* Parse attributes. This section eats a all input until 
           EOF, a > or a /> */
        attr = ParseXMLAttribute();
        while(attr != NULL) {
          attr->_next = node->_attributes;
          node->_attributes = attr;
          attr = ParseXMLAttribute();
        }

        /* This will eihter be a TOKEN_EOF, TOKEN_CLOSE_BRACKET, or a 
         * TOKEN_EMPTY_CLOSE_BRACKET */
        GetNextToken();
        
        /* Skip until '>', '/>' or EOF. This should really be an error, */
        /* but we are loose */
        assert(CurTokenType == TOKEN_EMPTY_CLOSE_BRACKET ||
               CurTokenType == TOKEN_CLOSE_BRACKET ||
               CurTokenType  == TOKEN_EOF, getMsgString(MSG_XMLPARSE_ERROR));
        
        if (CurTokenType == TOKEN_EMPTY_CLOSE_BRACKET) {
            GetNextToken();
            /* We are done with the sublevel - fall through to continue */
            /* parsing tags at the same level */            
        } else if (CurTokenType == TOKEN_CLOSE_BRACKET) {
            GetNextToken();
            
            /* Parse until end tag if found */
            node->_sub  = ParseXMLElement();
            
            if (CurTokenType == TOKEN_END_TAG) {
                /* Find closing bracket '>' for end tag */
                do {
                   GetNextToken();                   
                } while(CurTokenType != TOKEN_EOF && CurTokenType != TOKEN_CLOSE_BRACKET);
                GetNextToken();
            }  
        }               
        
        /* Continue parsing rest on same level */
        if (CurTokenType != TOKEN_EOF) {
                /* Parse rest of stream at same level */
                node->_next = ParseXMLElement();
        }
        return node;

    } else if (CurTokenType == TOKEN_PCDATA) {
        /* Create node for pcdata */
        node = CreateXMLNode(xmlPCDataType, strdup(CurTokenName));
        GetNextToken();
        return node;
    }

    /* Something went wrong. */
    return NULL;
}

/* Parses an XML attribute. */
static XMLAttribute* ParseXMLAttribute(void) {
    XMLAttribute* attr = NULL;
    char* q;
    char* name;
    char* value;
    char quoteChar;

    /* Skip whitespace etc. */
    SkipFilling();

    /* Check if we are done witht this attribute section */
    if (CurPos[0] == '\0' ||
        CurPos[0] == '>' ||
        CurPos[0] == '/' && CurPos[1] == '>') return NULL;

    /* Find end of name */
    q = CurPos;
    while(*q && !iswspace(*q) && *q !='=') q++;

    SetToken(TOKEN_UNKNOWN, CurPos, q);
    name = strdup(CurTokenName);

    /* Skip any whitespace */
    CurPos = q;
    CurPos = SkipFilling();
       
    /* Next character must be '=' for a valid attribute */
    if (*CurPos != '=') {
       /* This is really an error. We ignore this, and just try to parse an attribute
          out of the rest of the string */
       return ParseXMLAttribute();            
    }   
        
    CurPos++;
    CurPos = SkipWhiteSpace(CurPos);
    /* Parse CDATA part of attribute */
    if ((*CurPos == '\"') || (*CurPos == '\'')) {
        quoteChar = *CurPos;
        q = ++CurPos;
        while(*q != '\0' && *q != quoteChar) q++;
        SetToken(TOKEN_CDATA, CurPos, q);
        CurPos = q + 1;
    } else {
        q = CurPos;
        while(*q != '\0' && !iswspace(*q)) q++;
        SetToken(TOKEN_CDATA, CurPos, q);
        CurPos = q;
    }        

    value = strdup(CurTokenName);
    attr  = CreateXMLAttribute(name, value);        
    
    return attr;
}


void FreeXMLDocument(XMLNode* root) {
  if (root == NULL) return;
  FreeXMLDocument(root->_sub);
  FreeXMLDocument(root->_next);
  FreeXMLAttribute(root->_attributes);
  free(root->_name);
  free(root);
}

static void FreeXMLAttribute(XMLAttribute* attr) {
  if (attr == NULL) return;
  free(attr->_name);
  free(attr->_value);
  FreeXMLAttribute(attr->_next);
  free(attr);
}

/* Find element at current level with a given name */
XMLNode* FindXMLChild(XMLNode* root, char* name) {
  if (root == NULL) return NULL;

  if (root->_type == xmlTagType && strcmp(root->_name, name) == 0) {
    return root;
  } 

  return FindXMLChild(root->_next, name);
}

/* Search for an attribute with the given name and returns the contents. Returns NULL if
 * attribute is not found
 */
char* FindXMLAttribute(XMLAttribute* attr, char* name) {
  if (attr == NULL) return NULL;
  if (strcmp(attr->_name, name) == 0) return attr->_value;
  return FindXMLAttribute(attr->_next, name);
}


void PrintXMLDocument(XMLNode* node, int indt) {
  if (node == NULL) return;

  if (node->_type == xmlTagType) {
     printf("\n");
     indent(indt);
     printf("<%s", node->_name);
     PrintXMLAttributes(node->_attributes);
     if (node->_sub == NULL) {      
       printf("/>\n");
     } else {
       printf(">", node->_name);
       PrintXMLDocument(node->_sub, indt + 1);
       indent(indt);
       printf("</%s>", node->_name);       
     }
  } else {          
    printf("%s", node->_name);
  }
  PrintXMLDocument(node->_next, indt);
}

static void PrintXMLAttributes(XMLAttribute* attr) {
  if (attr == NULL) return;

  printf(" %s=\"%s\"", attr->_name, attr->_value);
  PrintXMLAttributes(attr->_next);
}

static void indent(int indt) {
    int i;  
    for(i = 0; i < indt; i++) {
        printf("  "); 
    }
}

char *CDStart = "<![CDATA[";
char *CDEnd = "]]>";


static char* SkipPCData(char *p) {
    char *end = strstr(p, CDEnd);
    if (end != NULL) {
	return end+sizeof(CDEnd);
    }
    return (++p);
}

static int IsPCData(char *p) {
    return (strncmp(CDStart, p, sizeof(CDStart)) == 0);
}

