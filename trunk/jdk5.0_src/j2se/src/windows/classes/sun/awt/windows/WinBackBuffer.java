/*
 * @(#)WinBackBuffer.java	1.12 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Component;
import java.awt.GraphicsConfiguration;
import java.awt.image.ColorModel;
import java.awt.image.VolatileImage;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.SurfaceData;

public class WinBackBuffer extends SunVolatileImage {

    /**
     * Create an image for an attached surface
     */
    public WinBackBuffer(Component c, Win32SurfaceData parentData) {
        super(c, c.getWidth(), c.getHeight(), parentData);
    }

    protected VolatileSurfaceManager createSurfaceManager(Object context) {
        return new WinBackBufferSurfaceManager(this, context);
    }

    public Win32BackBufferSurfaceData getHWSurfaceData() {
        SurfaceData sd = surfaceManager.getDestSurfaceData();
        if (sd instanceof Win32BackBufferSurfaceData) {
            return (Win32BackBufferSurfaceData)sd;
        } else {
            return null;
        }
    }

    private class WinBackBufferSurfaceManager
        extends WinVolatileSurfaceManager
    {
        public WinBackBufferSurfaceManager(SunVolatileImage vImg,
                                           Object context)
        {
            super(vImg, context);
        }

        protected Win32OffScreenSurfaceData createAccelSurface() {
            GraphicsConfiguration gc = vImg.getGraphicsConfig();
            ColorModel cm = getDeviceColorModel();
	    Win32SurfaceData parent = (Win32SurfaceData)context;
            return
                Win32BackBufferSurfaceData.createData(vImg.getWidth(),
                                                      vImg.getHeight(),
                                                      cm, gc, vImg, parent);
        }

        public void displayChanged() {
	    SurfaceData oldData;
            // Recreate java object with new display parameters
	    lostSurface = true;
	    if (sdAccel != null) {
		oldData = sdAccel;
		sdAccel = null;
		oldData.invalidate();
	    }
	    if (sdCurrent != null) {
		oldData = sdCurrent;
		sdCurrent = null;
		oldData.invalidate();
	    }

	    vImg.updateGraphicsConfig();
            sdAccel = sdCurrent = initAcceleratedSurface();
        }
    }
}
