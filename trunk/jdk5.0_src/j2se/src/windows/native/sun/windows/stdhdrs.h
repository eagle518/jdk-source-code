/*
 * @(#)stdhdrs.h	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _STDHDRS_H_
#define _STDHDRS_H_

// standard Windows and C headers
#define VC_EXTRALEAN	/* speeds compilation */
#ifndef STRICT
#define STRICT /* forces explicit typedef's for windows.h */
#endif
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <tchar.h>
// Don't #include <new> because that makes awt.dll dependent on
// msvcp50.dll. Instead, we've replicated the parts of <new>
// we need in alloc.h.
// #include <new>

extern "C" {

// standard Java headers
#include <jni.h>
#include <jni_util.h>

} // extern "C"

#endif // _STDHDRS_H_
