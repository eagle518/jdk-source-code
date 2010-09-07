/*
 * @(#)WGLVolatileSurfaceManager.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Component;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.ImageCapabilities;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.DisplayChangedListener;
import sun.awt.Win32GraphicsEnvironment;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.VolatileSurfaceManager;
import sun.awt.windows.WComponentPeer;
import sun.java2d.SurfaceData;

public class WGLVolatileSurfaceManager
    extends VolatileSurfaceManager
    implements DisplayChangedListener
{
    private boolean accelerationEnabled;

    public WGLVolatileSurfaceManager(SunVolatileImage vImg, Object context) {
        super(vImg, context);

	accelerationEnabled = (vImg.getTransparency() == Transparency.OPAQUE);

        Win32GraphicsEnvironment ge = (Win32GraphicsEnvironment)
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        ge.addDisplayChangedListener(this);
    }

    protected boolean isAccelerationEnabled() {
        return accelerationEnabled;
    }

    /**
     * Create a pbuffer-based SurfaceData object (or init the backbuffer
     * of an existing window if this is a double buffered GraphicsConfig).
     *
     * REMIND: the commented-out regions in this method are left here for
     *         testing purposes (the code will eventually be used to handle
     *         VolatileImages stored in the OGL backbuffer)...
     */    
    protected SurfaceData initAcceleratedSurface() {
        SurfaceData sData;
        Component comp = vImg.getComponent();
        WComponentPeer peer =
            (comp != null) ? (WComponentPeer)comp.getPeer() : null;

        try {
            boolean forceback = false;
            if (context instanceof Boolean) {
                forceback = ((Boolean)context).booleanValue();
	    }

            if (forceback) {
                // peer must be non-null in this case
                //WGLWindowSurfaceData sd =
                //    (WGLWindowSurfaceData)peer.getSurfaceData();
                //VolatileImage backbuffer = sd.getBackBuffer();
                //if (backbuffer != null) {
                //    backbuffer.flush();
                //}
                sData = WGLSurfaceData.createData(peer, true);
                //sd.setBackBuffer(this);
            } else {
                //boolean useback = false;
                //WGLGraphicsConfig gc =
                //    (WGLGraphicsConfig)vImg.getGraphicsConfig();
                //if (peer != null) {
                //    WGLWindowSurfaceData sd =
                //        (WGLWindowSurfaceData)peer.getSurfaceData();
                //    Rectangle peerbounds = peer.getBounds();
                //    useback = (gc.isDoubleBuffered() &&
                //               sd.getBackBuffer() == null &&
                //               vImg.getWidth() <= peerbounds.getWidth() &&
                //               vImg.getHeight() <= peerbounds.getHeight());
                //}

                //if (useback) {
                //    sData = WGLSurfaceData.createData(peer, false);
                //} else {
                    WGLGraphicsConfig gc =
                        (WGLGraphicsConfig)vImg.getGraphicsConfig();
                    ColorModel cm = gc.getColorModel();
                    sData =
                        WGLSurfaceData.createData(gc,
                                                  vImg.getWidth(),
                                                  vImg.getHeight(),
                                                  cm, vImg,
                                                  OGLSurfaceData.PBUFFER);
                //}
            }
        } catch (NullPointerException ex) {
            sData = null;
        } catch (OutOfMemoryError er) {
            sData = null;
        }

        return sData;
    }

    protected boolean isConfigValid(GraphicsConfiguration gc) {
        return ((gc == null) || (gc == vImg.getGraphicsConfig()));
    }

    public void flush() {
	lostSurface = true;
	WGLSurfaceData oldSD = (WGLSurfaceData)sdAccel;
	sdAccel = null;
	if (oldSD != null) {
	    oldSD.flush();
	}
    }

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

    public void paletteChanged() {
	lostSurface = true;
    }

    public ImageCapabilities getCapabilities() {
        if (isAccelerationEnabled()) {
            if (!(imageCaps instanceof WGLImageCaps)) {
                imageCaps = new WGLImageCaps();
	    }
	}
        return super.getCapabilities();
    }

    private class WGLImageCaps extends DefaultImageCapabilities {
        private WGLImageCaps() {
	}
        public boolean isTrueVolatile() {
            return isAccelerated();
        }
    }
}
