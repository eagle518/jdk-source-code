/*
 * @(#)JarFacade.h	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

typedef struct _jarAttribute {
    char* name;
    char* value;
    struct _jarAttribute* next;
} jarAttribute;


/* Returns a list of attributes */
jarAttribute* readAttributes(const char* jarfile);

/* Frees attribute list */
void freeAttributes(jarAttribute* attributes);

/* Gets the attribute by name */
char* getAttribute(const jarAttribute* attributes, const char* name);



