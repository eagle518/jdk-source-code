/*
 * @(#)XLayerProtocol.java	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.X11;

public interface XLayerProtocol {

    final static int LAYER_NORMAL = 0,
        LAYER_ALWAYS_ON_TOP = 1;

    boolean supportsLayer(int layer);
    void setLayer(XWindowPeer window, int layer);
}
