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
/*   $XConsortium: FileSB.h /main/12 1995/07/14 10:33:32 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFSelect_h
#define _XmFSelect_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Type definitions for FileSB resources: */


typedef void (*XmQualifyProc)( Widget, XtPointer, XtPointer) ;
typedef void (*XmSearchProc)( Widget, XtPointer) ;



/* Class record constants */

externalref WidgetClass xmFileSelectionBoxWidgetClass;

typedef struct _XmFileSelectionBoxClassRec * XmFileSelectionBoxWidgetClass;
typedef struct _XmFileSelectionBoxRec      * XmFileSelectionBoxWidget;

#define XmNenableFsbPickList "enableFsbPickList"
#define XmCEnableFsbPickList "EnableFsbPickList"

#ifndef XmIsFileSelectionBox
#define XmIsFileSelectionBox(w) (XtIsSubclass((w),xmFileSelectionBoxWidgetClass))
#endif


/********    Public Function Declarations    ********/

extern Widget XmFileSelectionBoxGetChild( 
                        Widget fs,
#if NeedWidePrototypes
                        unsigned int which) ;
#else
                        unsigned char which) ;
#endif /* NeedWidePrototypes */
extern void XmFileSelectionDoSearch( 
                        Widget fs,
                        XmString dirmask) ;
extern Widget XmCreateFileSelectionBox( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;
extern Widget XmCreateFileSelectionDialog( 
                        Widget ds_p,
                        String name,
                        ArgList fsb_args,
                        Cardinal fsb_n) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmFSelect_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
