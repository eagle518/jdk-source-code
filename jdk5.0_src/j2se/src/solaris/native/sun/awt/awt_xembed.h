/*
 * @(#)awt_xembed.h	1.1 03/08/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_XEMBED_H_
#define _AWT_XEMBED_H_

#ifndef HEADLESS

#include "awt_p.h"

extern void init_xembed();
extern void xembed_eventHandler(XEvent *event);
extern void requestXEmbedFocus(struct FrameData * wdata);
extern void install_xembed(Widget client, struct FrameData* wdata);
extern void deinstall_xembed(struct FrameData* wdata);
extern Boolean isXEmbedActive(struct FrameData * wdata);
extern Boolean isXEmbedActiveByWindow(Window client);
extern Boolean isXEmbedApplicationActive(struct FrameData * wdata);

#endif
#endif
