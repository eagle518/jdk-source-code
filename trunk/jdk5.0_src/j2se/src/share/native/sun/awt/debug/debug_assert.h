/*
 * @(#)debug_assert.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(_DASSERT_H)
#define _DASSERT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "debug_util.h"

#if defined(DEBUG)

#define DASSERT(_expr) \
	if ( !(_expr) ) { \
	    DAssert_Impl( #_expr, __FILE__, __LINE__); \
	} else { \
	}
	
#define DASSERTMSG(_expr, _msg) \
	if ( !(_expr) ) { \
	    DAssert_Impl( (_msg), __FILE__, __LINE__); \
	} else { \
	}

/* prototype for assert function */
typedef void (*DASSERT_CALLBACK)(const char * msg, const char * file, int line);

extern void DAssert_Impl(const char * msg, const char * file, int line);
extern void DAssert_SetCallback( DASSERT_CALLBACK pfn );

#else /* DEBUG not defined */

#define DASSERT(_expr)
#define DASSERTMSG(_expr, _msg)

#endif /* if defined(DEBUG) */

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* _DASSERT_H */
