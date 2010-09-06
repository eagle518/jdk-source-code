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
/*   $XConsortium: SeparatoG.h /main/11 1995/07/13 17:58:45 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*  Separator Gadget  */
#ifndef _XmSeparatorGadget_h
#define _XmSeparatorGadget_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsSeparatorGadget
#define XmIsSeparatorGadget(w) XtIsSubclass(w, xmSeparatorGadgetClass)
#endif /* XmIsSeparator */

externalref WidgetClass xmSeparatorGadgetClass;

typedef struct _XmSeparatorGadgetClassRec * XmSeparatorGadgetClass;
typedef struct _XmSeparatorGadgetRec      * XmSeparatorGadget;
typedef struct _XmSeparatorGCacheObjRec   * XmSeparatorGCacheObject;


/********    Public Function Declarations    ********/

extern Widget XmCreateSeparatorGadget( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSeparatorGadget_h */
/* DON'T ADD STUFF AFTER THIS #endif */
