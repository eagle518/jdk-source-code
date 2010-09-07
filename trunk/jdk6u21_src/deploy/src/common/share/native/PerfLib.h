/*
 * @(#)PerfLib.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _PERF_LIB_H_
#define _PERF_LIB_H_

#ifdef WIN32
#    ifdef PERFLIB_EXPORTS
#        define PERFLIB_API __declspec(dllexport)
#    else
#        define PERFLIB_API __declspec(dllimport)
#    endif    // PERFLIB_EXPORTS
#else
#    define PERFLIB_API
#endif    // WIN32

#endif    // _PERF_LIB_H_
