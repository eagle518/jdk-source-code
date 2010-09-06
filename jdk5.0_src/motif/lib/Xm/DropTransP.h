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
/*   $XConsortium: DropTransP.h /main/11 1995/07/14 10:31:56 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDropTransferP_h
#define _XmDropTransferP_h

#include <Xm/DropTrans.h>
#include <Xm/XmP.h>


#ifdef __cplusplus
extern "C" {
#endif

/*  DropTransfer class structure  */

typedef Widget (*XmDropTransferStartTransferProc)(Widget,
	ArgList, Cardinal);
typedef void (*XmDropTransferAddTransferProc)(Widget,
	XmDropTransferEntry, Cardinal);

typedef struct _XmDropTransferClassPart
{
	XmDropTransferStartTransferProc	start_drop_transfer;
	XmDropTransferAddTransferProc	add_drop_transfer;
	XtPointer extension;
} XmDropTransferClassPart;

/*  Full class record declaration */

typedef struct _XmDropTransferClassRec
{
   ObjectClassPart        object_class;
   XmDropTransferClassPart dropTransfer_class;
} XmDropTransferClassRec;

externalref XmDropTransferClassRec xmDropTransferClassRec;


typedef struct _XmDropTransferListRec {
	XmDropTransferEntry	transfer_list;
	Cardinal		num_transfers;
} XmDropTransferListRec, * XmDropTransferList;


/*  The DropTransfer instance record  */

typedef struct _XmDropTransferPart
{
    XmDropTransferEntry		drop_transfers;
    Cardinal			num_drop_transfers;
    Atom			selection;
    Widget			dragContext;
    Time			timestamp;
    Boolean			incremental;
    Window			source_window;
    unsigned int		tag;
    XtSelectionCallbackProc 	transfer_callback;
    unsigned char		transfer_status;

    Atom 			motif_drop_atom;
    
    XmDropTransferList		drop_transfer_lists;
    Cardinal			num_drop_transfer_lists;
    Cardinal			cur_drop_transfer_list;
    Cardinal			cur_xfer;
    Atom *			cur_targets;
    XtPointer *			cur_client_data;
    XtIntervalId		timer;
} XmDropTransferPart;

/*  Full instance record declaration  */

typedef struct _XmDropTransferRec
{
	ObjectPart	object;
	XmDropTransferPart dropTransfer;
} XmDropTransferRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDropTransferP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
