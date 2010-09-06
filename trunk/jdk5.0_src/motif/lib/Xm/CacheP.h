/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
/*   $XConsortium: CacheP.h /main/11 1995/07/14 10:12:51 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCacheP_h
#define _XmCacheP_h


#ifndef MOTIF12_HEADERS
#include <Xm/GadgetP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A few convenience macros */

#define ClassCacheHead(cp)	((cp)->cache_head)
#define ClassCacheCopy(cp)	((cp)->cache_copy)
#define ClassCacheCompare(cp)	((cp)->cache_compare)
#define CacheDataPtr(p)		((XtPointer)&((XmGadgetCacheRef*)p)->data)
#define DataToGadgetCache(p)	((char*)p - XtOffsetOf(XmGadgetCacheRef,data))


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */

/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $XConsortium: CacheP.h /main/cde1_maint/2 1995/08/18 18:52:11 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/GadgetP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* a few convenience macros */

#define ClassCacheHead(cp) \
	((cp)->cache_head)

#define ClassCacheCopy(cp) \
	((cp)->cache_copy)

#define ClassCacheCompare(cp) \
	((cp)->cache_compare)

#define CacheDataPtr(p) \
	((XtPointer)&((XmGadgetCacheRef*)p)-> data)

#define DataToGadgetCache(p)	((char *)p - XtOffsetOf(XmGadgetCacheRef, data))


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmCacheDelete() ;
extern void _XmCacheCopy() ;
extern XtPointer _XmCachePart() ;

#else

extern void _XmCacheDelete( 
                        XtPointer data) ;
extern void _XmCacheCopy( 
                        XtPointer src,
                        XtPointer dest,
                        size_t size) ;
extern XtPointer _XmCachePart( 
                        XmCacheClassPartPtr cp,
                        XtPointer cpart,
                        size_t size) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmCacheP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

