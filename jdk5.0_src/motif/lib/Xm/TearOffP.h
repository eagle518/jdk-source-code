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
/* $XConsortium: TearOffP.h /main/8 1995/10/25 20:21:05 cde-sun $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmTearOffP_h
#define _XmTearOffP_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmExcludedParentPaneRec
{
   short pane_list_size;
   Widget *pane;
   short num_panes;
} XmExcludedParentPaneRec;

/* Solaris 2.7 bugfix #4085003 : 1 line */
externalref XmExcludedParentPaneRec _XmExcludedParentPane;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmTearOffP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
