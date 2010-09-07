/*
 * @(#)EncodingSupport_md.c	1.4 04/06/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <langinfo.h>
#include <iconv.h>

/* Routines to convert back and forth between Platform Encoding and UTF-8 */

/* Error and assert macros */
#define UTF_ERROR(m) utfError(__FILE__, __LINE__,  m)
#define UTF_ASSERT(x) ( (x)==0 ? UTF_ERROR("ASSERT ERROR " #x) : (void)0 )
#define UTF_DEBUG(x)  

/* Global variables */
static iconv_t iconvToPlatform		= (iconv_t)-1;
static iconv_t iconvFromPlatform	= (iconv_t)-1;

/*
 * Error handler
 */
static void
utfError(char *file, int line, char *message)
{
    (void)fprintf(stderr, "UTF ERROR [\"%s\":%d]: %s\n", file, line, message);
    abort();
}

/* 
 * Initialize all utf processing.
 */
static void 
utfInitialize(void)
{
    char *codeset;
 
    /* Set the locale from the environment */
    (void)setlocale(LC_ALL, "");

    /* Get the codeset name */
    codeset = (char*)nl_langinfo(CODESET);  
    if ( codeset == NULL || codeset[0] == 0 ) {
        UTF_DEBUG(("NO codeset returned by nl_langinfo(CODESET)\n"));
	return;
    }
    
    UTF_DEBUG(("Codeset = %s\n", codeset));
    
    /* If we don't need this, skip it */
    if (strcmp(codeset, "UTF-8") == 0 || strcmp(codeset, "utf8") == 0 ) {
        UTF_DEBUG(("NO iconv() being used because it is not needed\n"));
	return;
    }

    /* Open conversion descriptors */
    iconvToPlatform   = iconv_open(codeset, "UTF-8");
    if ( iconvToPlatform == (iconv_t)-1 ) {
	UTF_ERROR("Failed to complete iconv_open() setup");
    }
    iconvFromPlatform = iconv_open("UTF-8", codeset);
    if ( iconvFromPlatform == (iconv_t)-1 ) {
	UTF_ERROR("Failed to complete iconv_open() setup");
    }
}

/*
 * Terminate all utf processing
 */
static void 
utfTerminate(void)
{
    if ( iconvFromPlatform!=(iconv_t)-1 ) {
	(void)iconv_close(iconvFromPlatform);
    }
    if ( iconvToPlatform!=(iconv_t)-1 ) {
	(void)iconv_close(iconvToPlatform);
    }
    iconvToPlatform   = (iconv_t)-1;
    iconvFromPlatform = (iconv_t)-1;
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
    
    if ( ic != (iconv_t)-1 ) {
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
static int 
utf8ToPlatform(char *utf8, int len, char *output, int outputMaxLen)
{
    return iconvConvert(iconvToPlatform, utf8, len, output, outputMaxLen);
}

/*
 * Convert Platform Encoding to UTF-8.
 *    Returns length or -1 if output overflows.
 */
static int 
platformToUtf8(char *str, int len, char *output, int outputMaxLen)
{
    return iconvConvert(iconvFromPlatform, str, len, output, outputMaxLen);
}

int 
convertUft8ToPlatformString(char* utf8_str, int utf8_len, char* platform_str, int platform_len) {
    if (iconvToPlatform ==  (iconv_t)-1) {
	utfInitialize();
    }
    return utf8ToPlatform(utf8_str, utf8_len, platform_str, platform_len);
}


