/*
 *  @(#)StdAfx.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// stdafx.h  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__3651881A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
#define AFX_STDAFX_H__3651881A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_

#ifdef XP_WIN

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#endif

#else  // For Unix
extern void trace_adapter(const char*);
#define TRACE(msg) trace_adapter(msg)
#define ASSERT(expr) assert(expr)

#endif

// Disable jni.h headers in Mozilla -- this causes conflict with the jni.h 
// in the JDK/JRE
#ifndef JNI_H
#define JNI_H
#endif

// Make sure ref-count tracing is not enabled in XPCOM
#define NO_BUILD_REFCNT_LOGGING
#define NSCAP_DISABLE_TEST_DONTQUERY_CASES

#include "JDSmartPtr.h"
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_STDAFX_H__3651881A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED)
