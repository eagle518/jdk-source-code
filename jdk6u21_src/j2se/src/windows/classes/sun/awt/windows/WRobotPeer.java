/*
 * @(#)WRobotPeer.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.peer.RobotPeer;

class WRobotPeer extends WObjectPeer implements RobotPeer
{
    WRobotPeer() {
        create();
    }
    WRobotPeer(GraphicsDevice screen) {
        create();
    }

    private synchronized native void _dispose();

    protected void disposeImpl() {
        _dispose();
    }
      
    public native void create();
    public native void mouseMoveImpl(int x, int y);
    public void mouseMove(int x, int y) {
        mouseMoveImpl(x, y);
    }
    public native void mousePress(int buttons);
    public native void mouseRelease(int buttons);
    public native void mouseWheel(int wheelAmt);

    public native void keyPress( int keycode );
    public native void keyRelease( int keycode );

    public int getRGBPixel(int x, int y) {
        return getRGBPixelImpl(x, y);
    }
    public native int getRGBPixelImpl(int x, int y);
    
    public int [] getRGBPixels(Rectangle bounds) {
        int pixelArray[] = new int[bounds.width*bounds.height];
        getRGBPixels(bounds.x, bounds.y, bounds.width, bounds.height, pixelArray);
        return pixelArray;
    }

    private native void getRGBPixels(int x, int y, int width, int height, int pixelArray[]);
}
