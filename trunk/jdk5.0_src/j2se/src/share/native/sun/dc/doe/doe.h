/*
 * @(#)doe.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)doe.h 3.3 97/11/18
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DOE_H
#define _DOE_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "dtypes.h"

/*
 *  The Ductus Operating Environment (DOE) is a collection of conventions,
 *  data structures and procedures, the common foundation upon which
 *  Ductus graphics libraries (DGLs) are constructed. DOE provides DGLs
 *  with the following services and mechanisms:
 *
 *	* Dynamic memory allocation: DGLs generally need to allocate and
 *	  deallocate memory during execution.
 *
 *	* File access: DGLs may need occasionally to access files, most
 *	  likely to retrieve fonts, images, etc. +++ it is separate from
 *	  everything else though
 *
 *	* Thread synchonization: Because DGLs are expected to work in
 *	  a multithreaded environment - to be "multithread safe" - a
 *	  basic mechanism for thread synchronization is required to
 *	  avoid conflicting access to shared resources.
 *
 *  The preceeding services are provided by all operating systems, albeit
 *  in different forms. DOE isolates the library from the specific details
 *  of the actual OS and allows one implementation of a DGL to be used in
 *  all cases. But DOE goes beyond presenting different OSs under a common
 *  interface. It offers also the following facilities, generally absent
 *  from OSs:
 *
 *	* An object model: In most domains - and certainly in the domain
 *	  of graphics - the object paradigm is far more appropriate as a
 *	  way of expression than the procedural paradigm. Yet DGLs are
 *	  C-procedural by definition.  The object model consists of
 *	  conventions and mechanisms to retain what we consider the most
 *	  important benefits of the object paradigm under the constrains
 *	  impossed by a procedural interface.
 *
 *	* (future) A mechanism for "cacheing": Caches are redundant
 *	  pieces of data used to improve the library's response time
 *	  to certain requests.  The computations that take place in
 *	  graphics libraries - for example, the rasterization of a
 *	  character's outline - are frequently time-consuming and
 *	  likely to be invoked repeatedly.  It is then particularily
 *	  important to "cache" their results - in our example, the
 *	  rasterized character.
 *
 *	* (future) Automatic memory management: Provides a
 *	  garbage-collection mechanism by which the memory occupied by
 *	  inaccessible objects and - if need be - caches is
 *	  automatically recovered.
 *
 *	* (future, maybe) A "notification" mechanism.
 */

#ifdef DEBUG			/* at least placed under DEBUG */
#define DEBUG_MT
#define DEBUG_MEM
#endif

/*
 *  ------------------------------------
 *	ENVIRONMENT
 *  ------------------------------------
 */

typedef struct	doeEData_*	doeE;

typedef void	(*doeReporterF)(doeE, char* msg);
typedef void	(*doeSetError)(doeE env, char** msgs, ixx index);
typedef void	(*doeNoMemory)(doeE env);

typedef struct doeEData_ {
    char**	msgtable;
    ixx		msgindex;
    doeSetError	errorfunc;
    doeNoMemory	nomemfunc;

    doeReporterF
		reporter;

    char*	thname;

    void*	octxt;	/* own context */
    void*	pctxt;	/* parent context */
} doeEData;

extern doeE	doeE_make();
extern void	doeE_destroy(doeE);
#define doeE_setReporter(env, r)	 (env)->reporter = r;
#define doeE_getReporter(env)		((env)->reporter)
#define doeE_setThName(env, n)		 (env)->thname = n;
#define doeE_getThName(env)		((env)->thname)
#define	doeE_setPCtxt(env, ctxt)	 (env)->pctxt = (void*)ctxt
#define doeE_getPCtxt(env)		((env)->pctxt)
#define	doeE_setOCtxt(env, ctxt)	 (env)->octxt = (void*)ctxt
#define doeE_getOCtxt(env)		((env)->octxt)

/*
 *  ------------------------------------
 *	DYNAMIC MEMORY ALLOCATION
 *  ------------------------------------
 */

extern	void*		doeMem_malloc	(doeE env, i32 bytes);
extern	void*		doeMem_realloc	(doeE env, void* mem, i32 bytes);
extern	void		doeMem_free	(doeE env, void* mem);


/*
 *  ------------------------------------
 *	THREAD SYNCHRONIZATION
 *  ------------------------------------
 */

typedef struct doeSemaData_*	doeSema;
extern doeSema	doeSema_create(doeE env, i32 count);
extern void	doeSema_destroy(doeE env, doeSema);
extern void	doeSema_post(doeE env, doeSema);
extern void	doeSema_wait(doeE env, doeSema);

typedef struct doeMutexData_*	doeMutex;
extern doeMutex	doeMutex_create(doeE env);
extern void	doeMutex_destroy(doeE env, doeMutex);
extern void	doeMutex_lock(doeE env, doeMutex);
extern void	doeMutex_unlock(doeE env, doeMutex);


/*
 *  ------------------------------------
 *	ERROR HANDLING
 *  ------------------------------------
 */

#define	doeError_occurred(env)	((env)->msgtable != NULL)
#define doeError_reset(env)	(env)->msgtable = NULL
#define doeError_set(env, msgs, index)	(env)->errorfunc(env, msgs, index)
#define doeError_setNoMemory(env) (env)->nomemfunc(env)

#ifdef	__cplusplus
}
#endif

#endif	/* _DOE_H */
