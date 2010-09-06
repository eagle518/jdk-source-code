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
/* $XConsortium: ProtocolsP.h /main/10 1995/07/13 17:42:13 drk $ */
/* (c) Copyright 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmProtocolsP_h
#define _XmProtocolsP_h

#include <Xm/Protocols.h>
#include <Xm/ExtObjectP.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmProtocolClassPart {
    XtPointer	extension;
} XmProtocolClassPart;

typedef struct _XmProtocolClassRec {
    ObjectClassPart	object_class;
    XmExtClassPart	ext_class;
    XmProtocolClassPart	protocol_class;
} XmProtocolClassRec, *XmProtocolObjectClass;

typedef struct _XmProtocolPart {
    XtCallbackRec	pre_hook, post_hook;
    XtCallbackList	callbacks;
    Atom		atom;
    Boolean		active;
} XmProtocolPart, *XmProtocolPartPtr;

typedef struct _XmProtocolRec {
    ObjectPart			object;
    XmExtPart			ext;
    XmProtocolPart		protocol;
} XmProtocolRec, *XmProtocol, **XmProtocolList;

#ifndef XmIsProtocol
#define XmIsProtocol(w)	XtIsSubclass(w, xmProtocolObjectClass)
#endif /* XmIsProtocol */

/* Class record constants */

externalref XmProtocolClassRec 	xmProtocolClassRec;
externalref WidgetClass xmProtocolObjectClass;

typedef struct _XmProtocolMgrRec{
    Atom		property;
    XmProtocolList 	protocols;
    Cardinal		num_protocols;
    Cardinal		max_protocols;
}XmProtocolMgrRec, *XmProtocolMgr, **XmProtocolMgrList;


typedef struct _XmAllProtocolsMgrRec{
  XmProtocolMgrList	protocol_mgrs;
  Cardinal		num_protocol_mgrs;
  Cardinal		max_protocol_mgrs;
  Widget		shell;
}XmAllProtocolsMgrRec, *XmAllProtocolsMgr;
    

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmProtocolsP_h */
