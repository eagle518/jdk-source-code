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
/* $XConsortium: TransferI.h /main/6 1996/10/16 16:57:37 drk $ */
#ifndef _XmTransferI_H
#define _XmTransferI_H

#include <Xm/TransferT.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/* Structure which is stored for each transfer request.  This   */
/* is where XmTransferValue will keep the data for the internal */
/* wrapper. 							*/
/****************************************************************/

#define TB_NONE		    0
#define TB_IGNORE	    1
#define TB_INTERNAL   	    2

typedef struct {
  XtPointer		next;
  XtPointer		client_data;
  XtPointer		location_data;
  int			flags;
  Atom			target;
  XtCallbackProc	selection_proc;
} TransferBlockRec, *TransferBlock;

/****************************************************************/
/* This structure forms the context block for each transfer_id. */
/* These structures are chained to allow for multiple           */
/* concurrent outstanding data transfers.                       */
/****************************************************************/

typedef struct {
  XtPointer	next;
  XtPointer	prev;
  Widget	widget;
  Atom		selection;
  Atom		real_selection;
  XtEnum	op;
  int		count;
  int		outstanding;
  int		flags;
  int		status;
  Widget	drag_context;
  Widget	drop_context;
  XmSelectionFinishedProc	*doneProcs;
  int		numDoneProcs;
  XtCallbackProc		auto_proc;
  XtPointer	client_data;
  XmDestinationCallbackStruct	*callback_struct;
  TransferBlock last;
  TransferBlock requests;
} TransferContextRec, *TransferContext;

enum{ TC_NONE = 0, TC_FLUSHED = 1, TC_CALLED_WIDGET = 2,
      TC_CALLED_CALLBACKS = 4, TC_EXITED_DH = 8, 
      TC_DID_DELETE = 16, TC_IN_MULTIPLE = 32 };

/****************************************************************/
/* The convertProc has a small context block which is           */
/* setup by the source calls and deleted in the loseProc        */
/****************************************************************/

typedef struct {
  long		op;	/* Make it big so it can hold a variety of data */
  int		flags;
  long		itemid;
  XtPointer	location_data;
  XtPointer	client_data;
  Widget	drag_context;
} ConvertContextRec, *ConvertContext;

enum{ CC_NONE = 0};

/****************************************************************/
/* Need to keep a count of outstanding SNAPSHOT requests.       */
/****************************************************************/

typedef struct {
  long		outstanding;
  Atom		distinguisher;
} SnapshotRequestRec, *SnapshotRequest;

/* Internal functions */

extern char * _XmTextToLocaleText(Widget, XtPointer, Atom, int,
				  unsigned long, Boolean *);

extern void _XmConvertHandlerSetLocal(void);

extern Boolean _XmConvertHandler(Widget wid, Atom *selection, Atom *target, 
			  Atom *type, XtPointer *value, 
			  unsigned long *size, int *fmt);

extern Boolean _XmDestinationHandler(Widget wid, Atom selection, XtEnum op,
			      XmSelectionFinishedProc done_proc,
			      XtPointer location_data, Time time,
			      XSelectionRequestEvent *event);

extern void _XmConvertComplete(Widget wid, XtPointer value, 
			unsigned long size, int format, Atom type,
			XmConvertCallbackStruct *cs);

extern XmDestinationCallbackStruct
       *_XmTransferGetDestinationCBStruct(XtPointer);

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransferI_H */

