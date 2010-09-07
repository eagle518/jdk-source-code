/*
 * @(#)JarFacade.c	1.2 04/06/09 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>
#include "jni.h"

#include "manifest_info.h"

typedef struct {
    int attributeCount;
    char** attributes;
    char** values;
} iterationContext;

static void 
doAttribute(const char* name, const char* value, void* user_data) {
    int i;
    iterationContext* context = (iterationContext*) user_data;

    for (i=0; i<context->attributeCount; i++) {
	if (strcasecmp(name, context->attributes[i]) == 0) {
	    if (context->values[i] == NULL) {
		context->values[i] = strdup(value);
	    }
	    break;
	}
    }
}

int
parseJarFile(const char* name, int attributeCount, char** attributes, char**values) {
    iterationContext context;
    int i, rc;

    context.attributeCount = attributeCount;
    context.attributes = attributes;
    context.values = values;
    for (i=0; i<attributeCount; i++) {
	values[i] = NULL;
    }

    rc = manifest_iterate(name, doAttribute, (void*)&context);

    return (rc == 0) ? 0 : -1;    
}
