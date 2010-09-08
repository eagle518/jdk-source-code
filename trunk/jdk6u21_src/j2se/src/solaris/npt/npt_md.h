/*
 * @(#)npt_md.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Native Platform Toolkit */

#ifndef  _NPT_MD_H
#define _NPT_MD_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <link.h>

#define NPT_LIBNAME "libnpt.so"

#define NPT_INITIALIZE(pnpt,version,options)				\
    {									\
        void   *_handle;						\
        void   *_sym;							\
									\
        if ( (pnpt) == NULL ) NPT_ERROR("NptEnv* is NULL");		\
        *(pnpt) = NULL;							\
        _handle =  dlopen(NPT_LIBNAME, RTLD_LAZY);			\
        if ( _handle == NULL ) NPT_ERROR("Cannot open library");	\
        _sym = dlsym(_handle, "nptInitialize");				\
        if ( _sym == NULL ) NPT_ERROR("Cannot find nptInitialize");	\
        ((NptInitialize)_sym)((pnpt), version, (options));		\
        if ( (*(pnpt)) == NULL ) NPT_ERROR("Cannot initialize NptEnv");	\
        (*(pnpt))->libhandle = _handle;					\
    }

#define NPT_TERMINATE(npt,options)					\
    {									\
        void *_handle;							\
        void *_sym;							\
									\
        if ( (npt) == NULL ) NPT_ERROR("NptEnv* is NULL");		\
        _handle = (npt)->libhandle;					\
        _sym = dlsym(_handle, "nptTerminate");				\
        if ( _sym == NULL ) NPT_ERROR("Cannot find nptTerminate");	\
        ((NptTerminate)_sym)((npt), (options));				\
        if ( _handle != NULL ) (void)dlclose(_handle);			\
    }


#endif

