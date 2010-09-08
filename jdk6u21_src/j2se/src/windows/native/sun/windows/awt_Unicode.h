/*
 * @(#)awt_Unicode.h	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Unicode to ANSI string conversion macros, based on a slide from a
 * presentation by Asmus Freytag.  These must be macros, since the 
 * alloca() has to be in the caller's stack space.
 */

#ifndef AWT_UNICODE_H
#define AWT_UNICODE_H

#include <malloc.h>

// Get a Unicode string copy of a Java String object (Java String aren't
// null-terminated).
extern LPWSTR J2WHelper(LPWSTR lpw, LPWSTR lpj, int nChars);
extern LPWSTR J2WHelper1(LPWSTR lpw, LPWSTR lpj, int offset, int nChars);

extern LPWSTR JNI_J2WHelper1(JNIEnv *env, LPWSTR lpw, jstring jstr);

#define TO_WSTRING(jstr) \
   ((jstr == NULL) ? NULL : \
     (JNI_J2WHelper1(env, (LPWSTR) alloca((env->GetStringLength(jstr)+1)*2), \
		     jstr) \
    ))

#endif // AWT_UNICODE_H
