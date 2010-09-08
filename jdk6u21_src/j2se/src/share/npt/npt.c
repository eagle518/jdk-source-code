/*
 * @(#)npt.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jni.h"

#include "npt.h"

#include "utf.h"

static int
version_check(char *version)
{
    if ( version==NULL || strcmp(version, NPT_VERSION)!=0 ) {
        return 1;
    }
    return 0;
}

JNIEXPORT void JNICALL 
nptInitialize(NptEnv **pnpt, char *nptVersion, char *options)
{
    NptEnv *npt;

    (*pnpt) = NULL;

    if ( version_check(nptVersion) ) {
	NPT_ERROR("NPT version doesn't match");
	return;
    }

    npt = (NptEnv*)calloc(sizeof(NptEnv), 1);
    if ( npt == NULL ) {
	NPT_ERROR("Cannot allocate calloc space for NptEnv*");
	return;
    }

    if ( options != NULL ) {
	npt->options = strdup(options);
    }
    npt->utfInitialize 		= &utfInitialize;
    npt->utfTerminate 		= &utfTerminate;
    npt->utf8ToPlatform 	= &utf8ToPlatform;
    npt->utf8FromPlatform 	= &utf8FromPlatform;
    npt->utf8ToUtf16 		= &utf8ToUtf16;
    npt->utf16ToUtf8m 		= &utf16ToUtf8m;
    npt->utf16ToUtf8s 		= &utf16ToUtf8s;
    npt->utf8sToUtf8mLength 	= &utf8sToUtf8mLength;
    npt->utf8sToUtf8m 		= &utf8sToUtf8m;
    npt->utf8mToUtf8sLength 	= &utf8mToUtf8sLength;
    npt->utf8mToUtf8s 		= &utf8mToUtf8s;
    
    (*pnpt) = npt;
}

JNIEXPORT void JNICALL 
nptTerminate(NptEnv* npt, char *options)
{

    /* FIXUP: options? Check memory or something? */
    if ( npt->options != NULL ) {
	(void)free(npt->options);
    }
    (void)free(npt);
}

