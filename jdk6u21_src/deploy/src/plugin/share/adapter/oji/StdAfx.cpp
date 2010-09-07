/*
 * @(#)StdAfx.cpp	1.6 10/03/24 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// stdafx.cpp  by Stanley Man-Kit Ho
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
