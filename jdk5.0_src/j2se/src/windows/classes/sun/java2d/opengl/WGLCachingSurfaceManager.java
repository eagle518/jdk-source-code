/*
 * @(#)WGLCachingSurfaceManager.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.util.Hashtable;
import sun.awt.DisplayChangedListener;
import sun.awt.Win32GraphicsEnvironment;
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
    implements DisplayChangedListener
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

        GraphicsEnvironment ge = 
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        // We could have a HeadlessGE at this point, so double-check before
        // assuming anything
        if (ge instanceof Win32GraphicsEnvironment) {
            ((Win32GraphicsEnvironment)ge).addDisplayChangedListener(this);
        }
    }

    protected SurfaceData getAccelSurface(GraphicsConfiguration gc) {
        return accelSurfaces != null ?
            (SurfaceData)accelSurfaces.get(gc) : null;
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

    protected void initAcceleratedSurface(GraphicsConfiguration gc, 
                                          int width, int height)
    {
        synchronized (this) {
            try {
                sdAccel = getAccelSurface(gc);
                if (sdAccel == null) {
                    if (gc instanceof WGLGraphicsConfig) {
                        sdAccel = createAccelSurface(gc, width, height);
                    }

                    if (sdAccel != null) {
                        accelSurfaces.put(gc, sdAccel);
                    }
                }
            } catch (sun.java2d.InvalidPipeException e) {
                // Problems during creation.  Don't propagate the exception,
                // just set the hardware surface data to null; the software
                // surface data will be used in the meantime
                sdAccel = null;
            } catch (OutOfMemoryError er) {
                sdAccel = null;
            }
        }
    }

    protected SurfaceData createAccelSurface(GraphicsConfiguration gc,
                                             int width, int height)
    {
        return WGLSurfaceData.createData((WGLGraphicsConfig)gc,
                                         width, height,
                                         gc.getColorModel(), bImg,
                                         OGLSurfaceData.TEXTURE);
    }

    public void displayChanged() {
        if (!accelerationEnabled) {
            return;
        }
        synchronized (this) {
            if (sdAccel != null) {
                Hashtable oldAccelSurfaces = accelSurfaces;
                sdAccel = null;
                accelSurfaces = new Hashtable();
                if (oldAccelSurfaces != null) {
                    Object[] array = oldAccelSurfaces.values().toArray();
                    for (int i = 0; i < array.length; i++) {
                        ((SurfaceData)array[i]).invalidate();
                    }
                }
            }
        }
    }

    public void paletteChanged() {
        sdDefault.setNeedsBackup(true);
    }

    public synchronized void flush() {
        sdAccel = null;
        if (accelSurfaces != null) {
            Hashtable oldAccelSurfaces = accelSurfaces;
            accelSurfaces = new Hashtable();
            Object array[] = oldAccelSurfaces.values().toArray();
            for (int i = 0; i < array.length; i++) {
                ((WGLSurfaceData)array[i]).flush();
            }
	}
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
