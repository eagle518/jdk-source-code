/*
 * @(#)npt_md.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Native Platform Toolkit */

#ifndef  _NPT_MD_H
#define _NPT_MD_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define NPT_LIBNAME "npt.dll"

#define NPT_INITIALIZE(pnpt,version,options)				\
    {									\
        HINSTANCE jvm;							\
	void   *_handle;						\
        void   *_sym;							\
	char    buf[FILENAME_MAX+32];					\
	char   *lastSlash;						\
									\
        if ( (pnpt) == NULL ) NPT_ERROR("NptEnv* is NULL");		\
	_handle =  NULL;						\
        *(pnpt) = NULL;							\
	buf[0] = 0;							\
	jvm = LoadLibrary("jvm.dll");					\
        if ( jvm == NULL ) NPT_ERROR("Cannot find jvm.dll");		\
	GetModuleFileName(jvm, buf, FILENAME_MAX);			\
	lastSlash = strrchr(buf, '\\');					\
	if ( lastSlash != NULL ) {					\
	    *lastSlash = '\0';						\
	    (void)strcat(buf, "\\..\\");				\
	    (void)strcat(buf, NPT_LIBNAME);				\
	    _handle =  LoadLibrary(buf);				\
	}								\
        if ( _handle == NULL ) NPT_ERROR("Cannot open library");	\
        _sym = GetProcAddress(_handle, "nptInitialize"); 		\
        if ( _sym == NULL ) NPT_ERROR("Cannot find nptInitialize");	\
        ((NptInitialize)_sym)((pnpt), version, (options));		\
        if ( *(pnpt) == NULL ) NPT_ERROR("Cannot initialize NptEnv");	\
        (*(pnpt))->libhandle = _handle;					\
    }

#define NPT_TERMINATE(npt,options)					\
    {									\
        void *_handle;							\
        void *_sym;							\
									\
        if ( (npt) == NULL ) NPT_ERROR("NptEnv* is NULL");		\
        _handle = (npt)->libhandle;					\
        if ( _handle == NULL ) NPT_ERROR("npt->libhandle is NULL");	\
        _sym = GetProcAddress(_handle, "nptTerminate"); 		\
        if ( _sym == NULL ) NPT_ERROR("Cannot find nptTerminate");	\
        ((NptTerminate)_sym)((npt), (options));				\
        (void)FreeLibrary(_handle);					\
    }

#endif

