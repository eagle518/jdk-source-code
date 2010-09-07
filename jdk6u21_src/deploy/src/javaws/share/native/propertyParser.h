/*
 * @(#)propertyParser.h	1.16 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef PROPERTYPARSER_H
#define PROPERTYPARSER_H

/*
 * Configuration file
 */
typedef struct _propertyFileEntry { 
  char* key;
  char* value;
  char* read_value;
  struct _propertyFileEntry* next;
} PropertyFileEntry;

void storePropertyFile(char *, PropertyFileEntry*);
PropertyFileEntry* parsePropertyFile(char *filename, PropertyFileEntry* head);
PropertyFileEntry* parsePropertyStream(char *s, PropertyFileEntry* head);

/* Find an entry */
char* GetPropertyValue(PropertyFileEntry* head, char* key);

/* Free's all memory allocated by the structure */
void FreePropertyEntry(PropertyFileEntry* head);

/* Add (or modify) a property */
PropertyFileEntry *AddProperty(PropertyFileEntry *head, char *key, char *value);

/* Debugging */
void PrintPropertyEntry(PropertyFileEntry* entry);
void PrintSinglePropertyEntry(PropertyFileEntry* entry);

#endif


