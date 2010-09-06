/*
 * @(#)StdAfx.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// stdafx.cpp  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "StdAfx.h"

#ifdef XP_WIN
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#else
#include <stdlib.h>
#include <stdio.h>
void
trace_adapter(const char* msg) {
  if (getenv("JAVA_PLUGIN_ADAPTER_TRACE") == NULL)
    return;

  fprintf(stdout, "Adapter: %s", msg);

  return;
}
#endif
