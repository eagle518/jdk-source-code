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
/*   $XConsortium: Protocols.h /main/11 1995/07/13 17:41:53 drk $ */
/*
*  (c) Copyright 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmProtocols_h
#define _XmProtocols_h

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* should be in XmP.h */

#ifndef XmCR_WM_PROTOCOLS
#define XmCR_WM_PROTOCOLS 6666
#endif /* XmCR_WM_PROTOCOLS */

/* define the XM_PROTOCOLS atom for use in  routines */
#ifdef XA_WM_PROTOCOLS
#define XM_WM_PROTOCOL_ATOM(shell) XA_WM_PROTOCOLS
#else
#define XM_WM_PROTOCOL_ATOM(shell) \
    XInternAtom(XtDisplay(shell),"WM_PROTOCOLS",FALSE)
#endif /* XA_WM_PROTOCOLS */


#define XmAddWMProtocols(shell, protocols, num_protocols) \
      XmAddProtocols(shell, XM_WM_PROTOCOL_ATOM(shell), \
			 protocols, num_protocols)

#define XmRemoveWMProtocols(shell, protocols, num_protocols) \
      XmRemoveProtocols(shell, XM_WM_PROTOCOL_ATOM(shell), \
			protocols, num_protocols)

#define XmAddWMProtocolCallback(shell, protocol, callback, closure) \
      XmAddProtocolCallback(shell, XM_WM_PROTOCOL_ATOM(shell), \
			    protocol, callback, closure)

#define XmRemoveWMProtocolCallback(shell, protocol, callback, closure) \
  XmRemoveProtocolCallback(shell, XM_WM_PROTOCOL_ATOM(shell), \
			    protocol, callback, closure)

#define XmActivateWMProtocol(shell, protocol) \
      XmActivateProtocol(shell, XM_WM_PROTOCOL_ATOM(shell), protocol)

#define XmDeactivateWMProtocol(shell, protocol) \
      XmDeactivateProtocol(shell, XM_WM_PROTOCOL_ATOM(shell), protocol)

#define XmSetWMProtocolHooks(shell, protocol, pre_h, pre_c, post_h, post_c) \
      XmSetProtocolHooks(shell, XM_WM_PROTOCOL_ATOM(shell), \
			 protocol, pre_h, pre_c, post_h, post_c)


/********    Public Function Declarations    ********/

extern void XmAddProtocols( 
                        Widget shell,
                        Atom property,
                        Atom *protocols,
                        Cardinal num_protocols) ;
extern void XmRemoveProtocols( 
                        Widget shell,
                        Atom property,
                        Atom *protocols,
                        Cardinal num_protocols) ;
extern void XmAddProtocolCallback( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void XmRemoveProtocolCallback( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void XmActivateProtocol( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom) ;
extern void XmDeactivateProtocol( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom) ;
extern void XmSetProtocolHooks( 
                        Widget shell,
                        Atom property,
                        Atom proto_atom,
                        XtCallbackProc pre_hook,
                        XtPointer pre_closure,
                        XtCallbackProc post_hook,
                        XtPointer post_closure) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmProtocols_h */
