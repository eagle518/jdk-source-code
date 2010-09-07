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
/* $XConsortium: TransferP.h /main/5 1995/07/13 18:14:36 drk $ */
#ifndef _TransferP_H
#define _TransferP_H

#include <Xm/Transfer.h>

extern void XmeConvertMerge(XtPointer, Atom, int, unsigned long,
		     XmConvertCallbackStruct*);
extern Boolean XmePrimarySource(Widget, Time);
extern Boolean XmeNamedSource(Widget, Atom, Time);
extern Boolean XmeSecondarySource(Widget, Time);
extern void XmeSecondaryTransfer(Widget, Atom, XtEnum, Time);
extern Boolean XmeClipboardSource(Widget, XtEnum, Time);
extern Widget XmeDragSource(Widget, XtPointer, XEvent*, ArgList, Cardinal);

extern Boolean XmePrimarySink(Widget, XtEnum, XtPointer, Time);
extern Boolean XmeNamedSink(Widget, Atom, XtEnum, XtPointer, Time);
extern Boolean XmeSecondarySink(Widget, Time);
extern Boolean XmeClipboardSink(Widget, XtEnum, XtPointer);
extern void XmeDropSink(Widget, ArgList, Cardinal);

extern Atom *XmeStandardTargets(Widget, int, int*);
extern void XmeStandardConvert(Widget, XtPointer, XmConvertCallbackStruct*);
extern Atom XmeGetEncodingAtom(Widget);
extern void XmeTransferAddDoneProc(XtPointer,
				   XmSelectionFinishedProc);

#endif /* _TransferP_H */
