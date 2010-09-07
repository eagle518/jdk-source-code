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
/*   $XConsortium: BulletinB.h /main/12 1995/07/14 10:11:57 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmBulletinBoard_h
#define _XmBulletinBoard_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmBulletinBoardWidgetClass;

typedef struct _XmBulletinBoardClassRec * XmBulletinBoardWidgetClass;
typedef struct _XmBulletinBoardRec      * XmBulletinBoardWidget;


#ifndef XmIsBulletinBoard
#define XmIsBulletinBoard(w)  (XtIsSubclass (w, xmBulletinBoardWidgetClass))
#endif



/********    Public Function Declarations    ********/

extern Widget XmCreateBulletinBoard( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;
extern Widget XmCreateBulletinBoardDialog( 
                        Widget ds_p,
                        String name,
                        ArgList bb_args,
                        Cardinal bb_n) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmBulletinBoard_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
