/*
 * @(#)MOD_memory.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*

**
**	History	-
**	12/1/96		Created
**	2/5/97		Fixed bug with mod_realloc that did not free old pointer
**	7/17/97		Added compile time switch
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
**	3/5/98		Fixed mod_realloc so it doesn't walk over memory bounds
*/
/*****************************************************************************/
#include "X_API.h"

#if USE_MOD_API == TRUE
#include "MOD_mikmod.h"
//
//	mod_memory.c
//
//	Memory-manager interfaces for MOD code.
//

/*
void * mod_malloc(long size)
{
	return XNewPtr(size);
}

void mod_free(void* ptr)
{
	XDisposePtr(ptr);
}

void mod_memset(void *ptr, char c, long size)
{
	XSetMemory(ptr, size, c);
}
*/


void * mod_realloc(void* ptr, long size)
{
    XPTR	p;

    p = NULL;
    if (ptr)
	{
	    p = XNewPtr(size);
	    if (p)
		{
		    size = XGetPtrSize((XPTR)ptr);	// get old size
		    if (size)
			{
			    XBlockMove(ptr, p, size);
			}
		    else
			{
			    XDisposePtr(p);
			    p = NULL;	// can't get size, so fail
			}
		    XDisposePtr(ptr);
		}
	}
    return p;
}

#endif	// USE_MOD_API


