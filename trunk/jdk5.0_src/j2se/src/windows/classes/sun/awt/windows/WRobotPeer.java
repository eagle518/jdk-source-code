/*
 * @(#)WRobotPeer.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.peer.RobotPeer;

class WRobotPeer extends WObjectPeer implements RobotPeer
{
    private Point offset = new Point(0, 0);

    WRobotPeer() {
	create();
    }
    WRobotPeer(GraphicsDevice screen) {
        if ( screen != null ) {
            GraphicsConfiguration conf = screen.getDefaultConfiguration();
            if ( conf != null ) {
                offset = conf.getBounds().getLocation();
            }
        }
        create();
    }

    private synchronized native void _dispose();

    protected void disposeImpl() {
	_dispose();
    }
      
    public native void create();
    public native void mouseMoveImpl(int x, int y);
    public void mouseMove(int x, int y) {
        x += offset.x;
        y += offset.y;
        mouseMoveImpl(x, y);
    }
    public native void mousePress(int buttons);
    public native void mouseRelease(int buttons);
    public native void mouseWheel(int wheelAmt);

    public native void keyPress( int keycode );
    public native void keyRelease( int keycode );

    public int getRGBPixel(int x, int y) {
        x += offset.x;
        y += offset.y;
        return getRGBPixelImpl(x, y);
    }
    public native int getRGBPixelImpl(int x, int y);
    
    public int [] getRGBPixels(Rectangle bounds) {
        bounds.translate(offset.x, offset.y);
	int pixelArray[] = new int[bounds.width*bounds.height];
	getRGBPixels(bounds.x, bounds.y, bounds.width, bounds.height, pixelArray);
	return pixelArray;
    }

    private native void getRGBPixels(int x, int y, int width, int height, int pixelArray[]);
}
