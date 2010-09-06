/* $XConsortium: Transfer.h /main/5 1995/07/15 20:56:30 drk $ */
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

#ifndef _XmTransfer_H
#define _XmTransfer_H

#include <Xm/DragDrop.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Proc typedefs */

#define XmConvertCallbackProc 		XtCallbackProc
#define XmSelectionDoneProc   		XtSelectionDoneProc
#define XmCancelSelectionProc 		XtCancelConvertSelectionProc
#define XmDestinationCallbackProc	XtCallbackProc
#define XmSelectionCallbackProc		XtSelectionCallbackProc

/* Flags */

typedef enum { XmTRANSFER_DONE_SUCCEED = 0, XmTRANSFER_DONE_FAIL, 
	       XmTRANSFER_DONE_CONTINUE, XmTRANSFER_DONE_DEFAULT 
	     } XmTransferStatus;

/* Solaris 2.6 Motif diff bug # 2 lines */
enum { XmMOVE = (1L << 0), XmCOPY = (1L << 1), 
       XmLINK = (1L << 2), XmOTHER };

enum { XmSELECTION_DEFAULT = 0, XmSELECTION_INCREMENTAL,
       XmSELECTION_PERSIST, XmSELECTION_SNAPSHOT,
       XmSELECTION_TRANSACT };

enum { XmCONVERTING_NONE = 0, 
       XmCONVERTING_SAME = 1, 
       XmCONVERTING_TRANSACT = 2,
       XmCONVERTING_PARTIAL = 4 };

enum { XmCONVERT_DEFAULT = 0, XmCONVERT_MORE, 
       XmCONVERT_MERGE, XmCONVERT_REFUSE, XmCONVERT_DONE };

/* Callback structures */

typedef struct {
	int		reason;
	XEvent		*event;
	Atom		selection;
	Atom		target;
	XtPointer	source_data;
	XtPointer	location_data;
	int		flags;
	XtPointer	parm;
	int		parm_format;
	unsigned long	parm_length;
	Atom		parm_type;
	int		status;
	XtPointer	value;
	Atom		type;
	int		format;
	unsigned long	length;
} XmConvertCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	XtEnum		operation;	
	int		flags;
	XtPointer	transfer_id;
	XtPointer	destination_data;
	XtPointer	location_data;
	Time		time;
} XmDestinationCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	Atom		target;
	Atom		type;
	XtPointer	transfer_id;
	int		flags;
	int		remaining;
	XtPointer	value;
	unsigned long	length;
	int		format;
} XmSelectionCallbackStruct;

typedef struct {
	int		reason;
  	XEvent		*event;
	Atom		selection;
	XtPointer	transfer_id;
	XmTransferStatus	status;
	XtPointer	client_data;
} XmTransferDoneCallbackStruct;

typedef void (*XmSelectionFinishedProc)(Widget, XtEnum,
					XmTransferDoneCallbackStruct*);

void XmTransferDone(XtPointer, XmTransferStatus);
void XmTransferValue(XtPointer, Atom, XtCallbackProc,
		     XtPointer,	Time);
void XmTransferSetParameters(XtPointer, XtPointer, int,	unsigned long, Atom);
void XmTransferStartRequest(XtPointer);
void XmTransferSendRequest(XtPointer, Time);

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransfer_H */
