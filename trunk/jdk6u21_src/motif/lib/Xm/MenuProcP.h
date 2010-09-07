/* $XConsortium: MenuProcP.h /main/4 1995/07/15 20:52:51 drk $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */
#ifndef _XmMenuProcP_h
#define _XmMenuProcP_h

#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _XmTranslRec
{
  XtTranslations translations;
  struct _XmTranslRec * next;
};

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmMenuProcP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
