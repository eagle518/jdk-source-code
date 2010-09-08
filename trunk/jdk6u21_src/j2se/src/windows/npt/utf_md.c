/*
 * @(#)utf_md.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "utf.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
   
/*
 * Initialize all utf processing.
 */
struct UtfInst * JNICALL
utfInitialize(char *options)
{
    struct UtfInst *ui;
    LANGID langID;
    LCID localeID;
    TCHAR strCodePage[7];       // ANSI code page id
        
    ui = (struct UtfInst*)calloc(sizeof(struct UtfInst), 1);
    
    /*
     * Get the code page for this locale
     */
    langID = LANGIDFROMLCID(GetUserDefaultLCID());  
    localeID = MAKELCID(langID, SORT_DEFAULT);
    if (GetLocaleInfo(localeID, LOCALE_IDEFAULTANSICODEPAGE, 
                      strCodePage, sizeof(strCodePage)/sizeof(TCHAR)) > 0 ) {
        ui->platformCodePage = atoi(strCodePage);
    } else {
        ui->platformCodePage = GetACP();
    }
    return ui;
}

/*
 * Terminate all utf processing
 */
void JNICALL
utfTerminate(struct UtfInst *ui, char *options)
{
    (void)free(ui);
}

/*
 * Get wide string  (assumes len>0)
 */
static WCHAR*
getWideString(UINT codePage, char* str, int len, int *pwlen)
{
    int wlen;
    WCHAR* wstr;

    /* Convert the string to WIDE string */
    wlen = MultiByteToWideChar(codePage, 0, str, len, NULL, 0);
    *pwlen = wlen;
    if (wlen <= 0) {
        UTF_ERROR(("Can't get WIDE string length"));
	return NULL;
    }
    wstr = (WCHAR*)malloc(wlen * sizeof(WCHAR));
    if (wstr == NULL) { 
        UTF_ERROR(("Can't malloc() any space"));
        return NULL;
    }
    if (MultiByteToWideChar(codePage, 0, str, len, wstr, wlen) == 0) {
        UTF_ERROR(("Can't get WIDE string"));
        return NULL;
    }
    return wstr;
}

/*
 * Convert UTF-8 to a platform string 
 */
int JNICALL
utf8ToPlatform(struct UtfInst *ui, jbyte *utf8, int len, char* output, int outputMaxLen) 
{
    int wlen;
    int plen;
    WCHAR* wstr;

    /* Negative length is an error */
    if ( len < 0 ) {
	return -1;
    }

    /* Zero length is ok, but we don't need to do much */
    if ( len == 0 ) {
	output[0] = 0;
	return 0;
    }

    /* Get WIDE string version (assumes len>0) */
    wstr = getWideString(CP_UTF8, (char*)utf8, len, &wlen);
    if ( wstr == NULL ) {
	return -1;
    }

    /* Convert WIDE string to MultiByte string */
    plen = WideCharToMultiByte(ui->platformCodePage, 0, wstr, wlen, 
			       output, outputMaxLen, NULL, NULL);   
    free(wstr);
    if (plen <= 0) {
	UTF_ERROR(("Can't convert WIDE string to multi-byte"));
	return -1;
    }
    output[plen] = '\0';
    return plen;
}

/*
 * Convert Platform Encoding to UTF-8.
 */
int JNICALL
utf8FromPlatform(struct UtfInst *ui, char *str, int len, jbyte *output, int outputMaxLen)
{
    int wlen;
    int plen;
    WCHAR* wstr;

    /* Negative length is an error */
    if ( len < 0 ) {
	return -1;
    }

    /* Zero length is ok, but we don't need to do much */
    if ( len == 0 ) {
	output[0] = 0;
	return 0;
    }

    /* Get WIDE string version (assumes len>0) */
    wstr = getWideString(ui->platformCodePage, str, len, &wlen);
    if ( wstr == NULL ) {
	return -1;
    }

    /* Convert WIDE string to UTF-8 string */
    plen = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, 
			       (char*)output, outputMaxLen, NULL, NULL);   
    free(wstr);
    if (plen <= 0) {
	UTF_ERROR(("Can't convert WIDE string to multi-byte"));
	return -1;
    }
    output[plen] = '\0';
    return plen;
}

