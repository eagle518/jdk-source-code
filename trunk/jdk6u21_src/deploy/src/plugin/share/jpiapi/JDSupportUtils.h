/*
 * @(#)JDSupportUtils.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  JDSupportUtils.h  by X.Lu
 * 
 *
 * Definetion of various data type and macros such as ISupports implementation
 */
#ifndef _JD_SUPPORTUTILS_H_
#define _JD_SUPPORTUTILS_H_

#include "JDData.h"
     

/* Error number, for detail error code, look for nsError.h in Mozilla code base */
#define JD_OK			 		    0
#define JD_NOINTERFACE		 	 	((JDresult) 0x80004002L)
#define JD_ERROR_OUT_OF_MEMORY   	((JDresult) 0x8007000eL)
#define JD_ERROR_NULL_POINTER    	((JDresult) 0x80004003L)
#define JD_ERROR_FAILURE  		 	((JDresult) 0x80004005L)
#define JD_ERROR_NOT_IMPLEMENTED 	((JDresult) 0x80004001L)
#define JD_ERROR_NO_AGGREGATION  	((JDresult) 0x80040110L)
#define JD_ERROR_NOT_INITIALIZED 	((JDresult) 0xC1F30001L)
#define JD_ERROR_ILLEGAL_VALUE   	((JDresult) 0x80070057L)
#define JD_ERROR_FACTORY_NOT_LOADED ((JDresult) 0x800401f8L)


#define JD_FALSE         0
#define JD_TRUE		 1
#define JD_FAILURE       1
#define JD_SUCCESS       0

#define JD_FAILED(_JDresult)	((_JDresult) & 0x80000000)
#define JD_SUCCEEDED(_JDresult) (!((_JDresult) & 0x80000000))

#ifdef XP_UNIX

#define JD_IMETHOD_(type)	virtual  type
#define JD_METHOD_(type)	type

#else

#define JD_IMETHOD_(type)	virtual  type __stdcall
#define JD_METHOD_(type)	type __stdcall

#endif

#define JD_IMETHOD		JD_IMETHOD_(JDresult)
#define JD_METHOD		JD_METHOD_(JDresult)
#define JD_IMETHODIMP_(type)	JD_METHOD_(type)
#define JD_IMETHODIMP		JD_METHOD

#define RETURN_IFNULL(_ptr)     \
    if (_ptr == NULL)           \
        return JD_ERROR_NULL_POINTER; 

#ifdef XP_UNIX
#include <string.h>

#ifndef JDStrcasecmp
#define JDStrcasecmp(a, b) strcasecmp(a,b)
#endif

#ifndef HIBYTE
#define HIBYTE(x)   ((((u_int)(x)) & 0xff00)>>8)
#endif

#ifndef LOBYTE
#define LOBYTE(x)   (((u_int)(x)) & 0x00ff)
#endif

#else

#ifndef JDStrcasecmp
#define JDStrcasecmp(a, b) lstrcmpi(a,b)
#endif

#endif
#endif /* _JD_SUPPORTUTILS_H_ */

