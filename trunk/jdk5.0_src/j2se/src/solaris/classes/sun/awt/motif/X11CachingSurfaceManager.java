/*
 * @(#)X11CachingSurfaceManager.java	1.27 04/03/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.IndexColorModel;
import java.util.Enumeration;
import java.util.Hashtable;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11GraphicsDevice;
import sun.awt.X11SurfaceData;
import sun.awt.image.CachingSurfaceManager;
import sun.java2d.DisposerTarget;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;
import sun.java2d.opengl.GLXGraphicsConfig;
import sun.java2d.opengl.GLXSurfaceData;
import sun.java2d.opengl.OGLSurfaceData;

/**
 * This CachingSurfaceManager variant is responsible for managing both
 * X11 and GLX surfaces (if available) on Unix platforms.  Since X11 and
 * GLX GraphicsConfigurations may co-exist under the same GraphicsDevice and
 * GraphicsEnvironment, it is important that we have one SurfaceManager that
 * is capable of caching either kind of surface, depending on the particular
 * GraphicsConfiguration.  For example, if we're attempting to cache a
 * system memory surface under an X11GraphicsConfig, we will create an X11
 * Pixmap surface.  Later, that same image could be rendered on a
 * GLXGraphicsConfig, so we could cache the software surface as an OpenGL
 * texture, if possible.
 */
public class X11CachingSurfaceManager
    extends CachingSurfaceManager
    implements DisposerTarget
{
    /**
     * Hashtable of bitmasks per graphics device.
     * Bitmask is 1 bit Pixmap, created on native level and 
     * based on software SD.
     * The bitmasks get updated when the image is changed.
     */
    private Hashtable bitmasks;
    private boolean isBitmask;
    private int transparency;
    private Object disposerReferent = new Object();

    public X11CachingSurfaceManager(BufferedImage bImg) {
	super(bImg);

        ColorModel cm = bImg.getColorModel();
        transparency = cm.getTransparency();
	isBitmask = (transparency == Transparency.BITMASK);

        if (isBitmask) {
            // 4673490: we can't easily handle ByteBinary data in
            // updateBitmask(), so we should avoid acceleration in this
            // situation
            if ((cm instanceof IndexColorModel) && (cm.getPixelSize() < 8)) {
                localAccelerationEnabled = false;
            }
        }
    }

    protected boolean isDestSurfaceAccelerated(SurfaceData destSD) {
	return (destSD instanceof GLXSurfaceData ||
                (destSD instanceof X11SurfaceData &&
                 X11SurfaceData.isAccelerationEnabled()));
    }

    protected SurfaceData getAccelSurface(GraphicsConfiguration gc) {
	return accelSurfaces != null ? 
	    (SurfaceData)accelSurfaces.get(gc) : null;
    }

    protected boolean isOperationSupported(SurfaceData dstData,
                                           CompositeType comp, 
                                           Color bgColor, boolean scale)
    {
        if (dstData instanceof GLXSurfaceData) {
            return (bgColor == null || transparency == Transparency.OPAQUE);
        }

        if (transparency == Transparency.TRANSLUCENT) {
            // we can't accelerate translucent images in X11
            return false;
        }

        if (bgColor != null && 
            bgColor.getTransparency() != Transparency.OPAQUE) {
            return false;
        }

        // we don't have X11 scale loops, so always use
        // software surface in case of scaling
        if (!scale) {
            if (!isBitmask) {
                // we save a read over the wire for compositing
                // operations by copying from the buffered image sd
                if (CompositeType.SrcOverNoEa.equals(comp) ||
                    CompositeType.SrcNoEa.equals(comp))
                {
                    return true;
                }
            } else {
                // for transparent images SrcNoEa+bgColor has the
                // same effect as SrcOverNoEa+bgColor, so we allow
                // copying from pixmap sd using accelerated blitbg loops:
                // SrcOver will be changed to SrcNoEa in DrawImage.blitSD
                if (CompositeType.SrcOverNoEa.equals(comp) || 
                    (CompositeType.SrcNoEa.equals(comp) && bgColor != null))
                {
                    return true;
                }
            }
        }

	return false;
    }

    protected void initAcceleratedSurface(GraphicsConfiguration gc,
                                          int width, int height)
    {
	try {
	    sdAccel = (SurfaceData)accelSurfaces.get(gc);
	    if (sdAccel == null) {
                if (gc instanceof GLXGraphicsConfig) {
                    sdAccel = createGLXSurface(gc, width, height);
                } else {
                    sdAccel = createX11Surface(gc, width, height);
                }

                if (sdAccel != null) {
                    accelSurfaces.put(gc, sdAccel);
                }
	    }
	} catch (NullPointerException ex) {
	    sdAccel = null;
	} catch (OutOfMemoryError er) {
	    sdAccel = null;
	}
    }

    protected SurfaceData createX11Surface(GraphicsConfiguration gc,
                                           int width, int height)
    {
        int bm = 0;

        if (isBitmask) {
            if (bitmasks == null) {
                bitmasks = new Hashtable();
            }
            Integer bmInt = (Integer)bitmasks.get(gc.getDevice());
            if (bmInt == null) {
                // create new bitmask for this device
                int screen = ((X11GraphicsDevice)gc.getDevice()).getScreen();
                bm = updateBitmask(sdDefault,
                                   0 /* means create a new one */, 
                                   screen, width, height);
                if (bm != 0) {
                    synchronized (bitmasks) {
                        bitmasks.put(gc.getDevice(), new Integer(bm));
                    }
                }
            } else {
                // already have a bitmask for this device
                bm = bmInt.intValue();
            }
        }

        return X11SurfaceData.createData((X11GraphicsConfig)gc,
                                         width, height,
                                         gc.getColorModel(),
                                         bImg, 0, bm);
    }

    protected SurfaceData createGLXSurface(GraphicsConfiguration gc,
                                           int width, int height)
    {
        return GLXSurfaceData.createData((GLXGraphicsConfig)gc,
                                         width, height,
                                         gc.getColorModel(), bImg,
                                         OGLSurfaceData.TEXTURE);
    }

    protected void copyDefaultToAccelerated() {
        // don't need to update the bitmasks unless sdDefault needs backup
        boolean bitmasksUpdateNeeded = sdDefault.needsBackup();
	super.copyDefaultToAccelerated();
	if (isBitmask && bitmasksUpdateNeeded &&
            (sdAccel != null) && (bitmasks != null))
        {
	    updateBitmasks();
	}
    }

    private native int updateBitmask(SurfaceData sd, int oldBitmask, 
                                     int screen, int width, int height);

    private void updateBitmasks() {
        int width = bImg.getWidth();
        int height = bImg.getHeight();
	synchronized (bitmasks) {
	    for (Enumeration keys = bitmasks.keys(); keys.hasMoreElements() ;) {
		X11GraphicsDevice gd = (X11GraphicsDevice)keys.nextElement();
		int bm = ((Integer)bitmasks.get(gd)).intValue();
		updateBitmask(sdDefault, bm, gd.getScreen(), 
                              width, height);
	    }
	}
    }

    public Object getDisposerReferent() {
	return disposerReferent;
    }
}
