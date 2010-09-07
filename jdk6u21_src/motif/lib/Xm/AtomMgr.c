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
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: AtomMgr.c /main/12 1995/07/14 10:10:19 drk $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#include <Xm/AtomMgr.h>
#include <Xm/XmP.h>
#include <X11/Xresource.h>
  
#ifdef XmInternAtom
#undef XmInternAtom
#endif

#ifdef XmGetAtomName
#undef XmGetAtomName
#endif


/*****************************************************************************
 *
 *  XmInternAtom()
 *
 ****************************************************************************/

Atom 
XmInternAtom(
        Display *display,
        String name,
#if NeedWidePrototypes
        int only_if_exists )
#else
        Boolean only_if_exists )
#endif /* NeedWidePrototypes */
{
  /* While not yet obsolete, this routine is not in favor.  Use */
  /* XInternAtom directly. */
  return XInternAtom(display, name, only_if_exists);
}

/*****************************************************************************
 *
 *  XmGetAtomName()
 *
 ****************************************************************************/
    
String 
XmGetAtomName(
        Display *display,
        Atom atom )
{
  /* While not yet obsolete, this routine is not in favor.  Use */
  /* XGetAtomName directly. */
  return XGetAtomName(display, atom);
}    
