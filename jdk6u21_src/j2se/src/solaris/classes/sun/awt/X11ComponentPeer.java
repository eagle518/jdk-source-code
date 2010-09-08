/*
 * @(#)X11ComponentPeer.java	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.Rectangle;
import java.awt.GraphicsConfiguration;
import java.awt.image.ColorModel;
import sun.java2d.SurfaceData;
import java.awt.Graphics;

public interface X11ComponentPeer {
    long getContentWindow();
    SurfaceData getSurfaceData();
    GraphicsConfiguration getGraphicsConfiguration();
    ColorModel getColorModel();
    Rectangle getBounds();
    Graphics getGraphics();
    Object getTarget();
}
