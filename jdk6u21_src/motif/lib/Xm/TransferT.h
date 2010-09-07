/* $XConsortium: TransferT.h /main/4 1995/07/15 20:56:34 drk $ */
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

#ifndef _XmTransferT_H
#define _XmTransferT_H

#include <Xm/Xm.h>
#include <Xm/TransferP.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTtransfer;

/* Trait structures and typedefs, place typedefs first */

/* Version 0: initial release. */

typedef struct _XmTransferTraitRec {
  int				version;		/* 0 */
  XmConvertCallbackProc		convertProc;
  XmDestinationCallbackProc	destinationProc;
  XmDestinationCallbackProc	destinationPreHookProc;
} XmTransferTraitRec, *XmTransferTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTransferT_H */
