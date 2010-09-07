/*
 * @(#)WinVolatileSurfaceManager.java	1.21 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Color;
import java.awt.Component;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.ImageCapabilities;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.DisplayChangedListener;
import sun.awt.Win32GraphicsConfig;
import sun.awt.Win32GraphicsDevice;
import sun.awt.Win32GraphicsEnvironment;
import sun.awt.WindowsFlags;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;

/**
 * Windows platform implementation of the VolatileSurfaceManager class.
 * This implementation has to handle the case of surfaceLoss due
 * to displayChange or other events.  The class attempts to create
 * and use a hardware-based SurfaceData object (Win32OffScreenSurfaceData).
 * If this object cannot be created or re-created as necessary, the
 * class falls back to a software-based SurfaceData object 
 * (BufImgSurfaceData) that will be used until the hardware-based
 * SurfaceData can be restored.
 */
public class WinVolatileSurfaceManager
    extends VolatileSurfaceManager
    implements DisplayChangedListener
{
    private boolean accelerationEnabled;
    
    public WinVolatileSurfaceManager(SunVolatileImage vImg, Object context) {
        super(vImg, context);
        Win32GraphicsEnvironment ge = (Win32GraphicsEnvironment)
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        ge.addDisplayChangedListener(this);
	/* We enable acceleration only if all of the following are true:
	 * - ddraw is enabled
	 * - ddraw offscreen surfaces are enabled
	 * - Either:
	 *    - the image is opaque OR
	 *    - the image is translucent and translucency acceleration
	 *      is enabled on this device
	 * There is no acceleration for bitmask images yet because the
	 * process to convert transparent pixels into ddraw colorkey
	 * values is not worth the effort and time.  We should eventually
	 * accelerate transparent images the same way we do translucent
	 * ones; through translucent textures (transparent pixels would
	 * simply have an alpha of 0).
	 */
	Win32GraphicsDevice gd = 
	    (Win32GraphicsDevice)vImg.getGraphicsConfig().getDevice();
	accelerationEnabled = 
	    WindowsFlags.isDDEnabled() &&
	    WindowsFlags.isDDOffscreenEnabled() &&
	    ((vImg.getTransparency() == Transparency.OPAQUE) ||
	     ((vImg.getTransparency() == Transparency.TRANSLUCENT) &&
	      WindowsFlags.isTranslucentAccelerationEnabled() &&
	      gd.isD3DEnabledOnDevice()));
    }

    protected Win32OffScreenSurfaceData createAccelSurface() {
	int transparency = vImg.getTransparency();
	ColorModel cm;
        GraphicsConfiguration gc = vImg.getGraphicsConfig();
	if (transparency != Transparency.TRANSLUCENT) {
	    // REMIND: This will change when we accelerate bitmask VImages.
	    // Currently, we can only reach here if the image is either
	    // opaque or translucent
	    cm = getDeviceColorModel();
	} else {
	    cm = gc.getColorModel(Transparency.TRANSLUCENT);
	}
	return Win32OffScreenSurfaceData.createData(vImg.getWidth(),
                                                    vImg.getHeight(),
                                                    cm, gc, vImg,
                                                    transparency);
    }

    protected boolean isAccelerationEnabled() {
        return accelerationEnabled;
    }

    /**
     * Create a vram-based SurfaceData object  
     */    
    protected SurfaceData initAcceleratedSurface() {
        SurfaceData sData;

	try {
	    sData = createAccelSurface();
	} catch (sun.java2d.InvalidPipeException e) {
	    // Problems during creation.  Don't propagate the exception, just
	    // set the hardware surface data to null; the software surface
	    // data will be used in the meantime
            sData = null;
	}
        return sData;
    }

    /**
     * Called from Win32OffScreenSurfaceData to notify us that our
     * accelerated surface has been lost.
     */
    public SurfaceData restoreContents() {
	acceleratedSurfaceLost();
	return super.restoreContents();
    }

    protected ColorModel getDeviceColorModel() {
        Win32GraphicsConfig gc = (Win32GraphicsConfig)vImg.getGraphicsConfig();
        return gc.getDeviceColorModel();
    }

    /**
     * Called from superclass to force restoration of this surface
     * during the validation process.  The method calls into the
     * hardware SurfaceData object to force the restore.
     */
    protected void restoreAcceleratedSurface() {
	((Win32OffScreenSurfaceData)sdAccel).restoreSurface();
    }

    /**
     * Called from Win32GraphicsEnv when there has been a display mode change.
     * Note that we simply invalidate hardware surfaces here; we do not
     * attempt to recreate or re-render them.  This is to avoid doing
     * rendering operations on the AWT-Windows thread, which tends to
     * get into deadlock states with the rendering thread.  Instead,
     * we just nullify the old surface data object and wait for a future
     * method in the rendering process to recreate the surface.
     */
    public void displayChanged() {
	if (!isAccelerationEnabled()) {
	    return;
	}
	lostSurface = true;
	if (sdAccel != null) {
	    // First, nullify the software surface.  This guards against
	    // using a SurfaceData that was created in a different
	    // display mode.
	    sdBackup = null;
	    sdCurrent = getBackupSurface();
	    // Now, invalidate the old hardware-based SurfaceData
	    SurfaceData oldData = sdAccel;
	    sdAccel = null;
	    oldData.invalidate();
	}
	// Update graphicsConfig for the vImg in case it changed due to
	// this display change event
	vImg.updateGraphicsConfig();
    }

    /**
     * When device palette changes, need to force a new copy
     * of the image into our hardware cache to update the 
     * color indices of the pixels (indexed mode only).
     */
    public void paletteChanged() {
	lostSurface = true;
    }

    public ImageCapabilities getCapabilities() {
        if (isAccelerationEnabled()) {
            if (!(imageCaps instanceof DDImageCaps)) {
                imageCaps = new DDImageCaps();
	    }
	}
        return super.getCapabilities();
    }

    private class DDImageCaps extends DefaultImageCapabilities {
        private DDImageCaps() {
	}
        public boolean isTrueVolatile() {
            return isAccelerated();
        }
    }

    /**
     * Releases any associated hardware memory for this image by
     * calling flush on sdAccel.  This method forces a lostSurface
     * situation so any future operations on the image will need to
     * revalidate the image first.
     */
    public void flush() {
	lostSurface = true;
	Win32OffScreenSurfaceData oldSD = 
	    (Win32OffScreenSurfaceData)sdAccel;
	sdAccel = null;
	if (oldSD != null) {
	    oldSD.flush();
	}
    }
}
