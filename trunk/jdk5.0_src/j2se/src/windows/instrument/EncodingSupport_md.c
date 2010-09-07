/*
 * @(#)EncodingSupport_md.c	1.3 04/06/09 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


/*
 * Convert UTF-8 to a platform string 
 */
int 
convertUft8ToPlatformString(char* utf8_str, int utf8_len, char* platform_str, int platform_len) {
    LANGID langID;
    LCID localeID;
    TCHAR strCodePage[7];	// ANSI code page id
    UINT codePage;
    int wlen, plen;
    WCHAR* wstr;
        
    /*
     * Get the code page for this locale
     */
    langID = LANGIDFROMLCID(GetUserDefaultLCID());  
    localeID = MAKELCID(langID, SORT_DEFAULT);
    if (GetLocaleInfo(localeID, LOCALE_IDEFAULTANSICODEPAGE, 
		      strCodePage, sizeof(strCodePage)/sizeof(TCHAR)) > 0 ) {
        codePage = atoi(strCodePage);
    } else {
        codePage = GetACP();
    }

    /*
     * To convert the string to platform encoding we must first convert
     * to unicode, and then convert to the platform encoding
     */
    plen = -1;
    wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str, utf8_len, NULL, 0);
    if (wlen > 0) {
        wstr = (WCHAR*)malloc(wlen * sizeof(WCHAR));
	if (wstr != NULL) {
    	    if (MultiByteToWideChar(CP_UTF8, 
                                    0, 
                                    utf8_str, 
                                    utf8_len, 
                                    wstr, wlen) > 0) {
     	        plen = WideCharToMultiByte(codePage, 
                                           0, 
                                           wstr, 
                                           wlen, 
                                           platform_str, 
                                           platform_len, 
                                           NULL, 
                                           NULL);   
	  	if (plen >= 0) {	
    	 	    platform_str[plen] = '\0';
		}
		free(wstr);
	    }
	}
    }
    return plen;
}
