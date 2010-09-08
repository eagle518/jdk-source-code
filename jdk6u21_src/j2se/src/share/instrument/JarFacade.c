/*
 * @(#)JarFacade.c	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>
#include <stdlib.h>

#include "jni.h"
#include "manifest_info.h"
#include "JarFacade.h"

typedef struct {
    jarAttribute* head;
    jarAttribute* tail;
} iterationContext;

static void 
doAttribute(const char* name, const char* value, void* user_data) {
    iterationContext* context = (iterationContext*) user_data;

    jarAttribute* attribute = (jarAttribute*)malloc(sizeof(jarAttribute));
    if (attribute != NULL) {
	attribute->name = strdup(name);
	if (attribute->name == NULL) {
	    free(attribute);
	} else {
	    attribute->value = strdup(value);
	    if (attribute->value == NULL) {
	        free(attribute->name);
	        free(attribute);
	    } else {
		attribute->next = NULL;
		if (context->head == NULL) {
		    context->head = attribute;
		} else {
		    context->tail->next = attribute;
		}
		context->tail = attribute;
	    }
	}

    }
}

/*
 * Return a list of attributes from the main section of the given JAR
 * file. Returns NULL if there is an error or there aren't any attributes.
 */
jarAttribute* 
readAttributes(const char* jarfile)
{	      
    int rc;
    iterationContext context = { NULL, NULL };

    rc = JLI_ManifestIterate(jarfile, doAttribute, (void*)&context);

    if (rc == 0) {
	return context.head;
    } else {
	freeAttributes(context.head);	
	return NULL;
    }
}


/* 
 * Free a list of attributes
 */
void 
freeAttributes(jarAttribute* head) {
    while (head != NULL) {
        jarAttribute* next = (jarAttribute*)head->next;
	free(head->name);
	free(head->value);
	free(head);
	head = next;
    }
}

/*
 * Get the value of an attribute in an attribute list. Returns NULL
 * if attribute not found. 
 */
char* 
getAttribute(const jarAttribute* attributes, const char* name) {
    while (attributes != NULL) {
	if (strcasecmp(attributes->name, name) == 0) {
	    return attributes->value;
	}
	attributes = (jarAttribute*)attributes->next;
    }
    return NULL;
}

