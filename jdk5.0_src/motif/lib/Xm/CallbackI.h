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
/*   $XConsortium: CallbackI.h /main/8 1995/07/14 10:13:23 drk $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmCallbackI_h
#define _XmCallbackI_h

#include "XmI.h"

#ifdef __cplusplus
extern "C" {
#endif


#define _XtCBCalling		1
#define _XtCBFreeAfterCalling	2

/* The internal callback list is a little header followed by an array of
 * XtCallbackRec structs.  Declaring the first element of the array with 
 * the header allows the compiler to take care of any alignment required.
 */
typedef struct internalCallbackRec {
  unsigned short count;
  char           is_padded;	/* contains NULL padding for external form */
  char           call_state;	/* combination of _XtCB{FreeAfter}Calling */
  XtCallbackRec  callbacks;	/* first XtCallback record declared here */
} InternalCallbackRec, *InternalCallbackList;


/********    Private Function Declarations    ********/

extern void _XmAddCallback(InternalCallbackList *callbacks,
			   XtCallbackProc callback,
			   XtPointer closure);
extern void _XmRemoveCallback(InternalCallbackList *callbacks,
			      XtCallbackProc callback,
			      XtPointer closure);
extern void _XmCallCallbackList(Widget widget,
				XtCallbackList callbacks,
				XtPointer call_data);
extern void _XmRemoveAllCallbacks(InternalCallbackList *callbacks);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCallbackI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
