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
/*   $XConsortium: AtomMgr.h /main/10 1995/07/14 10:10:31 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmAtomMgr_h
#define _XmAtomMgr_h


#ifndef MOTIF12_HEADERS

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* As of X11r5 XInternAtom is cached by Xlib, so we can use it directly. */

/* fix for bug 4129529 - leob */
extern String XmGetAtomName(Display *display, Atom atom);

#define XmInternAtom(display, name, only_if_exists) \
		XInternAtom(display, name, only_if_exists)
#define XmGetAtomName(display, atom) \
		XGetAtomName(display, atom)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

/* This macro name is confusing, and of unknown benefit.
 * #define XmNameToAtom(display, atom) \
 *      XmGetAtomName(display, atom)
 */

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
/*   $XConsortium: AtomMgr.h /main/cde1_maint/2 1995/08/18 18:50:42 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XM_ATOM_CACHE

#ifdef XM_ATOM_CACHE
/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Atom XmInternAtom() ;
extern String XmGetAtomName() ;

#else

extern Atom XmInternAtom( 
                        Display *display,
                        String name,
#if NeedWidePrototypes
                        int only_if_exists) ;
#else
                        Boolean only_if_exists) ;
#endif /* NeedWidePrototypes */
extern String XmGetAtomName( 
                        Display *display,
                        Atom atom) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/
#else /* XM_ATOM_CACHE */
#define XmInternAtom(display, name, only_if_exists) \
		XInternAtom(display, name, only_if_exists)
#define XmGetAtomName(display, atom) \
		XGetAtomName(display, atom)
#endif /* XM_ATOM_CACHE */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#define XmNameToAtom(display, atom) \
        XmGetAtomName(display, atom)

#endif /* MOTIF12_HEADERS */
#endif /* _XmAtomMgr_h */
