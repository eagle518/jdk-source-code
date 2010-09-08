/*
 * @(#)utf_md.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>
#include <iconv.h>

#include "utf.h"

/* Global variables */

/* 
 * Initialize all utf processing.
 */
struct UtfInst *JNICALL
utfInitialize(char *options)
{
    struct UtfInst *ui;
    char           *codeset;
 
    ui = (struct UtfInst*)calloc(sizeof(struct UtfInst), 1);
    ui->iconvToPlatform		= (void *)-1;
    ui->iconvFromPlatform	= (void *)-1;
    
    /* Set the locale from the environment */
    (void)setlocale(LC_ALL, "");

    /* Get the codeset name */
    codeset = (char*)nl_langinfo(CODESET);  
    if ( codeset == NULL || codeset[0] == 0 ) {
	return ui;
    }
    
    /* If we don't need this, skip it */
    if (strcmp(codeset, "UTF-8") == 0 || strcmp(codeset, "utf8") == 0 ) {
	return ui;
    }

    /* Open conversion descriptors */
    ui->iconvToPlatform   = iconv_open(codeset, "UTF-8");
    if ( ui->iconvToPlatform == (void *)-1 ) {
	UTF_ERROR("Failed to complete iconv_open() setup");
    }
    ui->iconvFromPlatform = iconv_open("UTF-8", codeset);
    if ( ui->iconvFromPlatform == (void *)-1 ) {
	UTF_ERROR("Failed to complete iconv_open() setup");
    }
    return ui;
}

/*
 * Terminate all utf processing
 */
void  JNICALL
utfTerminate(struct UtfInst *ui, char *options)
{
    if ( ui->iconvFromPlatform != (void *)-1 ) {
	(void)iconv_close(ui->iconvFromPlatform);
    }
    if ( ui->iconvToPlatform != (void *)-1 ) {
	(void)iconv_close(ui->iconvToPlatform);
    }
    ui->iconvToPlatform   = (void *)-1;
    ui->iconvFromPlatform = (void *)-1;
    (void)free(ui);
}

/*
 * Do iconv() conversion.
 *    Returns length or -1 if output overflows.
 */
static int 
iconvConvert(iconv_t ic, char *bytes, int len, char *output, int outputMaxLen)
{
    int outputLen = 0;
    
    UTF_ASSERT(bytes);
    UTF_ASSERT(len>=0);
    UTF_ASSERT(output);
    UTF_ASSERT(outputMaxLen>len);
    
    output[0] = 0;
    outputLen = 0;
    
    if ( ic != (iconv_t)(void *)-1 ) {
	int          returnValue;
	size_t       inLeft;
	size_t       outLeft;
	char        *inbuf;
	char        *outbuf;
	
	inbuf        = bytes;
	outbuf       = output;
	inLeft       = len;
	outLeft      = outputMaxLen;
	returnValue  = iconv(ic, (void*)&inbuf, &inLeft, &outbuf, &outLeft);
	if ( returnValue >= 0 && inLeft==0 ) {
	    outputLen = outputMaxLen-outLeft;
	    output[outputLen] = 0;
	    return outputLen;
	}

	/* Failed to do the conversion */
	return -1;
    }

    /* Just copy bytes */
    outputLen = len;
    (void)memcpy(output, bytes, len);
    output[len] = 0;
    return outputLen;
}

/*
 * Convert UTF-8 to Platform Encoding.
 *    Returns length or -1 if output overflows.
 */
int  JNICALL
utf8ToPlatform(struct UtfInst*ui, jbyte *utf8, int len, char *output, int outputMaxLen)
{
    /* Negative length is an error */
    if ( len < 0 ) {
        return -1;
    }

    /* Zero length is ok, but we don't need to do much */
    if ( len == 0 ) {
        output[0] = 0;
        return 0;
    }

    return iconvConvert(ui->iconvToPlatform, (char*)utf8, len, output, outputMaxLen);
}

/*
 * Convert Platform Encoding to UTF-8.
 *    Returns length or -1 if output overflows.
 */
int  JNICALL
utf8FromPlatform(struct UtfInst*ui, char *str, int len, jbyte *output, int outputMaxLen)
{
    /* Negative length is an error */
    if ( len < 0 ) {
        return -1;
    }

    /* Zero length is ok, but we don't need to do much */
    if ( len == 0 ) {
        output[0] = 0;
        return 0;
    }
    
    return iconvConvert(ui->iconvFromPlatform, str, len, (char*)output, outputMaxLen);
}

