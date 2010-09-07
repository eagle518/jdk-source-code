/*
 * @(#)X11ComponentPeer.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
}
