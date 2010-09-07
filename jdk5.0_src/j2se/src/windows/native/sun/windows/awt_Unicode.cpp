/*
 * @(#)awt_Unicode.cpp	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"

LPWSTR J2WHelper1(LPWSTR lpw, LPWSTR lpj, int offset, int nChars) {
    memcpy(lpw, lpj + offset, nChars*2);
    lpw[nChars] = '\0';
    return lpw;
}

LPWSTR JNI_J2WHelper1(JNIEnv *env, LPWSTR lpwstr, jstring jstr) {

    int len = env->GetStringLength(jstr);

    env->GetStringRegion(jstr, 0, len, lpwstr);
    lpwstr[len] = '\0';

    return lpwstr;
}

LPWSTR J2WHelper(LPWSTR lpw, LPWSTR lpj,  int nChars) {
    return J2WHelper1(lpw, lpj, 0, nChars);
}
