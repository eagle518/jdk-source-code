/*
 * @(#)WGLCachingSurfaceManager.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.util.Hashtable;
import sun.awt.image.CachingSurfaceManager;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;

/**
 * This is a WGL-specific CachingSurfaceManager that can cache a system
 * memory surface in an accelerated OpenGL texture (one per GraphicsConfig).
 * This class is separate from WinCachingSurfaceManager because when the OGL
 * pipeline is enabled, DD and D3D are disabled, so it is not necessary to
 * have one class that can manage all pipelines.
 */
public class WGLCachingSurfaceManager
    extends CachingSurfaceManager
{
    private int transparency;

    public WGLCachingSurfaceManager(BufferedImage bImg) {
        super(bImg);

        transparency = bImg.getColorModel().getTransparency();

        // Pre-load the accelerated surface upon image creation.
        if ((accelerationThreshold == 0) && localAccelerationEnabled) {
            GraphicsConfiguration gc = 
                GraphicsEnvironment.getLocalGraphicsEnvironment().
                    getDefaultScreenDevice().getDefaultConfiguration();
            initAcceleratedSurface(gc, bImg.getWidth(), bImg.getHeight());
        }
    }

    protected boolean isDestSurfaceAccelerated(SurfaceData destSD) {
        return (destSD instanceof WGLSurfaceData);
    }

    protected boolean isOperationSupported(SurfaceData dstData,
                                           CompositeType comp,
                                           Color bgColor, boolean scale)
    {
        return ((dstData instanceof WGLSurfaceData) &&
                (bgColor == null || transparency == Transparency.OPAQUE));
    }

    protected SurfaceData createAccelSurface(GraphicsConfiguration gc,
                                             int width, int height)
    {
	if (gc instanceof WGLGraphicsConfig) {
	    return WGLSurfaceData.createData((WGLGraphicsConfig)gc,
					     width, height,
					     gc.getColorModel(transparency),
                                             bImg, OGLSurfaceData.TEXTURE);
	}
	return null;
    }

    public String toString() {
        return new String("WGLCachingSurfaceManager@" +
                          Integer.toHexString(hashCode()) + 
                          " transparency: " + 
                          (transparency == Transparency.OPAQUE ? 
                               "OPAQUE" : 
                               transparency == Transparency.BITMASK ? 
                                   "BITMASK" : 
                                   "TRANSLUCENT"));
    }
}
