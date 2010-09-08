/*
 * @(#)awt_xembed.h	1.1 03/08/18
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_XEMBED_H_
#define _AWT_XEMBED_H_

#ifndef HEADLESS

#include "awt_p.h"

#define XEMBED_VERSION  0
#define XEMBED_MAPPED  (1 << 0)
/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY              0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define XEMBED_REQUEST_FOCUS         3
#define XEMBED_FOCUS_IN             4
#define XEMBED_FOCUS_OUT            5
#define XEMBED_FOCUS_NEXT           6
#define XEMBED_FOCUS_PREV           7
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON          10
#define XEMBED_MODALITY_OFF         11
#define XEMBED_REGISTER_ACCELERATOR     12
#define XEMBED_UNREGISTER_ACCELERATOR   13
#define XEMBED_ACTIVATE_ACCELERATOR     14
    
#define XEMBED_LAST_MSG XEMBED_ACTIVATE_ACCELERATOR

#define  NON_STANDARD_XEMBED_GTK_GRAB_KEY  108
#define NON_STANDARD_XEMBED_GTK_UNGRAB_KEY  109

// Sun internal special message, to resolve start race condition
#define _SUN_XEMBED_START  1119


//     A detail code is required for XEMBED_FOCUS_IN. The following values are valid:    
/* Details for  XEMBED_FOCUS_IN: */
#define XEMBED_FOCUS_CURRENT        0
#define XEMBED_FOCUS_FIRST          1
#define XEMBED_FOCUS_LAST           2    


extern void init_xembed();
extern void xembed_eventHandler(XEvent *event);
extern void requestXEmbedFocus(struct FrameData * wdata);
extern void install_xembed(Widget client, struct FrameData* wdata);
extern void deinstall_xembed(struct FrameData* wdata);
extern Boolean isXEmbedActive(struct FrameData * wdata);
extern Boolean isXEmbedActiveByWindow(Window client);
extern Boolean isXEmbedApplicationActive(struct FrameData * wdata);
extern void sendMessageHelper(Window window, int message, long detail, 
                              long data1, long data2);
extern void sendMessage(Window window, int message);
extern void xembed_traverse_out(struct FrameData * wdata, jboolean);
#endif
#endif
