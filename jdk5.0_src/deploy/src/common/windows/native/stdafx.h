/*
 * @(#)stdafx.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// stdafx.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#if !defined(AFX_STDAFX_H__7BAF58A5_AD52_4E97_B447_7BCCEE841454__INCLUDED_)
#define AFX_STDAFX_H__7BAF58A5_AD52_4E97_B447_7BCCEE841454__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#endif // !defined(AFX_STDAFX_H__7BAF58A5_AD52_4E97_B447_7BCCEE841454__INCLUDED)
