/*
 * @(#)D3DCachingSurfaceManager.java	1.2 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.d3d;

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
 * This is a D3D-specific CachingSurfaceManager that can cache a system
 * memory surface in an accelerated D3D texture (one per GraphicsConfig).
 * This class is separate from WinCachingSurfaceManager because when the D3D
 * pipeline is enabled, the default GDI pipeline is disabled, so it is not
 * necessary to have one class that can manage all pipelines.
 */
public class D3DCachingSurfaceManager
    extends CachingSurfaceManager
{
    private int transparency;

    public D3DCachingSurfaceManager(BufferedImage bImg) {
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
        return (destSD instanceof D3DSurfaceData);
    }

    protected boolean isOperationSupported(SurfaceData dstData,
                                           CompositeType comp,
                                           Color bgColor, boolean scale)
    {
        return ((dstData instanceof D3DSurfaceData) &&
                (bgColor == null || transparency == Transparency.OPAQUE));
    }

    protected SurfaceData createAccelSurface(GraphicsConfiguration gc,
                                             int width, int height)
    {
	if (gc instanceof D3DGraphicsConfig) {
	    return D3DSurfaceData.createData((D3DGraphicsConfig)gc,
					     width, height,
					     gc.getColorModel(transparency),
                                             bImg, D3DSurfaceData.TEXTURE);
	}
	return null;
    }

    public String toString() {
        return new String("D3DCachingSurfaceManager@" +
                          Integer.toHexString(hashCode()) + 
                          " transparency: " + 
                          (transparency == Transparency.OPAQUE ? 
                               "OPAQUE" : 
                               transparency == Transparency.BITMASK ? 
                                   "BITMASK" : 
                                   "TRANSLUCENT"));
    }

    @Override
    protected void restoreAcceleratedSurface() {
        if (sdAccel != null) {
            ((D3DSurfaceData)sdAccel).restoreSurface();
        }
    }
}
