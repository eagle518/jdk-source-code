/*
 * @(#)WinCachingSurfaceManager.java	1.22 04/03/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.image.DirectColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.WritableRaster;
import java.util.Hashtable;

import sun.awt.DisplayChangedListener;
import sun.awt.Win32GraphicsConfig;
import sun.awt.Win32GraphicsEnvironment;
import sun.awt.WindowsFlags;
import sun.awt.image.CachingSurfaceManager;
import sun.java2d.SurfaceData;
import sun.java2d.SunGraphics2D;
import sun.java2d.loops.CompositeType;

/**
 * This is a Windows-specific CachingSurfaceManager that can cache a system
 * memory surface (with a supported ColorModel) in one or more accelerated
 * surface (one surface per GraphicsDevice).  For example, a bitmask
 * transparent surface may be cached as a DirectDraw surface using a color
 * key value to specify the transparent areas, or a translucent surface
 * may be cached as a Direct3D texture with a full alpha channel.
 *
 * For bitmask transparent images, there are methods in this class that
 * assist in finding a color that DirectDraw can use to represent any
 * transparent pixels in the source image.
 *
 * Note that the only bitmask transparent images we currently support are
 * those with either:
 *   - an IndexColorModel with bitmask transparency or
 *   - a 25 bit (1888 RGB) DirectColorModel with bitmask transparency
 * Also note that we must punt if the destination has 8-bit depth since the
 * dithering will upset our "unused color" algorithms.
 */
public class WinCachingSurfaceManager
    extends CachingSurfaceManager
    implements DisplayChangedListener
{
    /**
     * Represents the maximum size (width * height) of an image that we should
     * scan for an unused color.  Any image larger than this would probably
     * require too much computation time.
     */
    private static final int MAX_SIZE = 65536;

    /**
     * The following constants determine the size of the histograms used when
     * searching for an unused color
     */
    private static final int ICM_HISTOGRAM_SIZE = 256;
    private static final int ICM_HISTOGRAM_MASK = ICM_HISTOGRAM_SIZE - 1;
    private static final int DCM_HISTOGRAM_SIZE = 1024;
    private static final int DCM_HISTOGRAM_MASK = DCM_HISTOGRAM_SIZE - 1;

    private int transparency;
    private Color transColor;

    static {
        if (accelerationEnabled) {
            // acceleration may have been disabled via the system property
            // checked in the superclass, so we should only check these
            // additional flags if acceleration is still enabled
            accelerationEnabled = (WindowsFlags.isDDEnabled() &&
                                   WindowsFlags.isDDOffscreenEnabled());
        }
    }

    public WinCachingSurfaceManager(BufferedImage bImg) {
        super(bImg);

        ColorModel cm = bImg.getColorModel();
        transparency = cm.getTransparency();

        if (localAccelerationEnabled) {
            localAccelerationEnabled = isValidColorModel(cm);
        }

	// Pre-load the accelerated surface upon image creation.
	// REMIND: may need a separate flag for this
	if ((accelerationThreshold == 0) && localAccelerationEnabled) {
	    GraphicsConfiguration gc = 
		GraphicsEnvironment.getLocalGraphicsEnvironment().
                    getDefaultScreenDevice().getDefaultConfiguration();
	    initAcceleratedSurface(gc, bImg.getWidth(), bImg.getHeight());
	}
        
        GraphicsEnvironment ge = 
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        // We could have a HeadlessGE at this point, so double-check before
        // assuming anything.
	// Also, no point in listening to display change events if
	// the image is never going to be accelerated.
        if (accelerationEnabled && localAccelerationEnabled && 
	    ge instanceof Win32GraphicsEnvironment) 
	{
	    ((Win32GraphicsEnvironment)ge).addDisplayChangedListener(this);
        }
    }

    /**
     * Returns true if a surface with the specified ColorModel can be
     * potentially cached in an accelerated surface.
     */
    private boolean isValidColorModel(ColorModel cm) {
        if (transparency == Transparency.OPAQUE) {
            return true;
        } else if (transparency == Transparency.BITMASK) {
            if (cm instanceof IndexColorModel) {
                return true;
            } else if ((cm instanceof DirectColorModel) &&
                       (cm.getPixelSize() == 25) &&
                       (cm.getTransferType() == DataBuffer.TYPE_INT) &&
                       ((bImg.getWidth() * bImg.getHeight()) <= MAX_SIZE)) 
            {
                return true;
            } else {
                return false;
            }
        } else { // (transparency == Transparency.TRANSLUCENT)
            return WindowsFlags.isTranslucentAccelerationEnabled();
        }
    }

    /**
     * Returns true if the default surface can be cached in an accelerated
     * surface with the given ColorModel.
     */
    private boolean isValidDeviceColorModel(ColorModel cm) {
        if (transparency == Transparency.OPAQUE) {
            return true;
        } else if (transparency == Transparency.BITMASK) {
            return (cm.getPixelSize() != 8);
        } else { // (transparency == Transparency.TRANSLUCENT)
            // REMIND: we can loosen this restriction if copies from any
            //         surface to a texture surface always work properly...
            return Win32GraphicsConfig.getTranslucentColorModel().equals(cm);
        }
    }
    
    protected SurfaceData getAccelSurface(GraphicsConfiguration gc) {
	return accelSurfaces != null ?
	    (SurfaceData)accelSurfaces.get(gc.getDevice()) : null;
    }
	
    protected boolean isDestSurfaceAccelerated(SurfaceData destSD) {
	return (((destSD instanceof Win32OffScreenSurfaceData) &&
		 !((Win32OffScreenSurfaceData)destSD).surfacePunted()) ||
	        (destSD instanceof Win32SurfaceData));
    }

    protected boolean isOperationSupported(SurfaceData dstData,
                                           CompositeType comp,
                                           Color bgColor, boolean scale)
    {
	if (transparency == Transparency.OPAQUE) {
	    // we save a read from video memory for compositing
	    // operations by copying from the buffered image sd
	    if (CompositeType.SrcOverNoEa.equals(comp) ||
		CompositeType.SrcNoEa.equals(comp))
            {
		// allow using accelerated surface for scale blits
		// if DD scaling is enabled
		return (!scale || Win32OffScreenSurfaceData.isDDScaleEnabled());
	    }
	} else if (transparency == Transparency.TRANSLUCENT) {
	    if ((CompositeType.SrcOverNoEa.equals(comp) || 
		 CompositeType.SrcOver.equals(comp)) && 
		bgColor == null) 
	    {
		return !scale;
	    }
	} else {
	    // We have accelerated loops only for blits with SrcOverNoEa
	    // (no blit bg loops or blit loops with SrcNoEa)
	    // We also have no accelerated loops from _BM to translucent VI.
	    if (CompositeType.SrcOverNoEa.equals(comp) && bgColor == null &&
		dstData.getTransparency() != Transparency.TRANSLUCENT) 
	    {
		return !scale;
	    }
	}
	return false;
    }

    /**
     * The number of times we unsuccessfully tried
     * to accelerate the surface.
     * Used to avoid infinite retries to accelerate
     * the surface
     */
    private int timesTried = 0;

    protected void initAcceleratedSurface(GraphicsConfiguration gc, 
                                          int width, int height)
    {
	synchronized (this) {
	    try {
		sdAccel = getAccelSurface(gc);
		if (sdAccel == null) {
                    ColorModel cm;
                    if (transparency == Transparency.TRANSLUCENT) {
                        // use software color model for textures
                        cm = bImg.getColorModel();
                    } else {
                        // otherwise, use device color model
                        cm = ((Win32GraphicsConfig)gc).getDeviceColorModel(); 
                    }

                    // only attempt to cache surface if color model is valid
                    if (isValidDeviceColorModel(cm)) {
                        sdAccel =
                            Win32OffScreenSurfaceData.createData(width, height,
                                                                 cm, gc, bImg,
                                                                 transparency);
                        if (sdAccel != null) {
                            accelSurfaces.put(gc.getDevice(), sdAccel);
                        }
                    }
		}
	    } catch (sun.java2d.InvalidPipeException e) {
		// Problems during creation.  Don't propagate the exception,
		// just set the hardware surface data to null; the software
                // surface data will be used in the meantime
		sdAccel = null;
	    }

	    if (sdAccel == null) {
		if (timesTried++ > 3) {
		    localAccelerationEnabled = false;
		}
	    } else {
		timesTried = 0;
	    }

	    if (sdAccel != null &&
                transparency == Transparency.BITMASK &&
		getTransparentPixelColor() == null)
            {
		// we can't cache on this hardware surface since
		// we were unable to find the color which could
		// be used as color key.
		flush();

		// REMIND: getTransparentPixelColor method will
		// set localAccelerationEnabled to false
		// if it can't find the unused color. This
		// prevents us from further attempts to cache this image 
		// on other hw surfaces even though they may
		// have unused pixels.
	    }
	}
    }

    public SurfaceData restoreContents() {
	if (accelerationEnabled) {
	    synchronized (this) {
		if (sdAccel != null) {
		    validate(sdAccel.getDeviceConfiguration());
		}
	    }
	}
	return super.restoreContents();
    }

    protected void restoreAcceleratedSurface() {
	synchronized (this) {
	    if (sdAccel != null) {
		((Win32OffScreenSurfaceData)sdAccel).restoreSurface();
	    }
	}
    }

    /**
     * Called from WToolkit when there has been a display mode change.
     * Note that we simply invalidate hardware surfaces here; we do not
     * attempt to recreate or re-render them.  This is to avoid doing
     * rendering operations on the AWT-Windows thread, which tends to
     * get into deadlock states with the rendering thread.  Instead,
     * we just nullify the old surface data object and wait for a future
     * method in the rendering process to recreate the surface and
     * copy the backup if appropriate.
     *
     * REMIND: probably need to re-verify that we can support
     *         the image in this new display mode
     */
    public void displayChanged() {
	if (!accelerationEnabled) {
	    return;
	}
	// REMIND: may be playing too safe here: invalidate all SD
	// even if they're on a different device
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

    /**
     * When device palette changes, need to force a new copy
     * of the image into our hardware cache to update the 
     * color indices of the pixels (indexed mode only).
     */
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
		((Win32OffScreenSurfaceData)array[i]).flush();
	    }
	}
    }

    protected Color getTransparentPixelColor() {
        if (!localAccelerationEnabled ||
            transparency != Transparency.BITMASK)
        {
            return null;
        } else {
            // REMIND: might be able to cache transColor
            transColor = setupTransparentPixel();
            
            if (transColor == null) {
                localAccelerationEnabled = false;
            }

            return transColor;
        }
    }

    /**
     * Attempts to find an unused pixel value in the image and if successful,
     * sets up the DirectDraw surface so that it uses this value as its
     * color key
     */
    protected Color setupTransparentPixel() {
        Color transColor = null;
        Integer transPixel = null;
        
        ColorModel cm = bImg.getColorModel();
        if (cm instanceof IndexColorModel) {
            transPixel = findUnusedPixelICM();
        } else if (cm instanceof DirectColorModel) {
            transPixel = findUnusedPixelDCM();
        }

        if (transPixel != null) {
            int ipixel = transPixel.intValue();
            ((Win32OffScreenSurfaceData)sdAccel).setTransparentPixel(ipixel);
            int rgb = sdAccel.rgbFor(ipixel);
            transColor = new Color(rgb);
        }

        return transColor;
    }

    /**
     * Attempts to find an unused pixel value in the color map of an
     * IndexColorModel.  If successful, it returns that value (in the 
     * ColorModel of the destination surface) or null otherwise.
     */
    private Integer findUnusedPixelICM() {
        IndexColorModel icm = (IndexColorModel)bImg.getColorModel();
        int mapsize = icm.getMapSize();
        int[] histogram = new int[ICM_HISTOGRAM_SIZE];
        int[] cmap = new int[mapsize];
        icm.getRGBs(cmap);

        // load up the histogram
        for (int i = 0; i < mapsize; i++) {
            int pixel = sdAccel.pixelFor(cmap[i]);
            histogram[pixel & ICM_HISTOGRAM_MASK]++;
        }

        // find an empty histo-bucket
        for (int j = 0; j < histogram.length; j++) {
            int value = histogram[j];
            if (value == 0) {
                return new Integer(j);
            }
        }

        return null;
    }

    /**
     * Attempts to find an unused pixel value in an image with a 
     * 25-bit DirectColorModel and a DataBuffer of TYPE_INT.  If successful, 
     * it returns that value (in the ColorModel of the destination surface) 
     * or null otherwise.
     */
    private Integer findUnusedPixelDCM() {
        DataBufferInt db = (DataBufferInt)bImg.getRaster().getDataBuffer();
        // REMIND: we need to offset the effects of the call to 
        // getDataBuffer() by re-enabling local acceleration (we could not
        // have entered this method if local acceleration was not already
        // enabled, so we can turn acceleration back on without side effects)
        localAccelerationEnabled = true;
        int[] pixels = db.getData();
        int[] histogram = new int[DCM_HISTOGRAM_SIZE];

        // load up the histogram
        // REMIND: we could possibly make this faster by keeping track of
        // the unique colors found, and only doing a pixelFor() when we come
        // across a new unique color
        for (int i = 0; i < pixels.length; i++) {
            int pixel = sdAccel.pixelFor(pixels[i]);
            histogram[pixel & DCM_HISTOGRAM_MASK]++;
        }

        // find an empty histo-bucket
        for (int j = 0; j < histogram.length; j++) {
            int value = histogram[j];
            if (value == 0) {
                return new Integer(j);
            }
        }

        return null;
    }

    /**
     * This method is called during the process of calling
     * Win32OffScreenSurfaceData.markSurfaceLost().  Note that we throw an
     * exception in this version of the method, so we should only call this
     * method for those situations in which we know that it is safe to
     * throw an exception.
     */
    public void acceleratedSurfaceLost() {
	throw new sun.java2d.InvalidPipeException("Managed surface lost");
    }

    public String toString() {
        return new String("WinCachingSurfaceManager@" +
			  Integer.toHexString(hashCode()) + 
                          " transparency: " + 
			  (transparency == Transparency.OPAQUE ? 
			     "OPAQUE" : 
			     transparency == Transparency.BITMASK ? 
			        "BITMASK" : 
			        "TRANSLUCENT"));
    }
}
