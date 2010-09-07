/*
 * @(#)syshead.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * SYSHEAD.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
 /*
  * Here is an example of what an embedded client can do instead
  * of including the regular ANSI headers.
  */


#ifdef YOUR_COMPANY
#include "Headers.h"
#define assert(h) Assert(h)
#include "MemoryManager.h"

/* Moved to CONFIG.H ...
 *    #define malloc(n)			AllocateTaggedMemoryNilAllowed(n,"t2k")
 *     #define free(p)				FreeTaggedMemory(p,"t2k")
 *     #define realloc(ptr, size)	ReallocateTaggedMemoryNilAllowed(ptr, size, "t2k");
 */
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#endif
