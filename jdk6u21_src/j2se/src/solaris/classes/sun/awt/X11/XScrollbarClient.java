/*
 * @(#)XScrollbarClient.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;

/**
* Interface for notifying a client that a scrollbar has changed.
* @see java.awt.event.AdjustmentEvent
*/
interface XScrollbarClient {
    public void notifyValue(XScrollbar obj, int type, int value, boolean isAdjusting);
    public Component getEventSource();
    public void repaintScrollbarRequest(XScrollbar scrollBar);
}
