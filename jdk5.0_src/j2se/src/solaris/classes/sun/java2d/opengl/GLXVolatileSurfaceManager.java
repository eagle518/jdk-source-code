/*
 * @(#)GLXVolatileSurfaceManager.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Component;
import java.awt.GraphicsConfiguration;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.X11ComponentPeer;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.SurfaceData;

public class GLXVolatileSurfaceManager extends VolatileSurfaceManager {

    private boolean accelerationEnabled;

    public GLXVolatileSurfaceManager(SunVolatileImage vImg, Object context) {
        super(vImg, context);
	accelerationEnabled = (vImg.getTransparency() == Transparency.OPAQUE);
    }

    protected boolean isAccelerationEnabled() {
        return accelerationEnabled;
    }

    /**
     * Create a pbuffer-based SurfaceData object (or init the backbuffer
     * of an existing window if this is a double buffered GraphicsConfig)
     */    
    protected SurfaceData initAcceleratedSurface() {
        SurfaceData sData;
        Component comp = vImg.getComponent();
        X11ComponentPeer peer =
            (comp != null) ? (X11ComponentPeer)comp.getPeer() : null;

        try {
            boolean forceback = false;
            if (context instanceof Boolean) {
                forceback = ((Boolean)context).booleanValue();
	    }

            if (forceback) {
                // peer must be non-null in this case
                //GLXWindowSurfaceData sd =
                //    (GLXWindowSurfaceData)peer.getSurfaceData();
                //VolatileImage backbuffer = sd.getBackBuffer();
                //if (backbuffer != null) {
                //    backbuffer.flush();
                //}
                sData = GLXSurfaceData.createData(peer, true);
                //sd.setBackBuffer(this);
            } else {
                //boolean useback = false;
                //if (peer != null) {
                    //GLXWindowSurfaceData sd =
                    //    (GLXWindowSurfaceData)peer.getSurfaceData();
                    //Rectangle peerbounds = peer.getBounds();
                    //useback = (graphicsConfig.isDoubleBuffered() &&
                               //sd.getBackBuffer() == null &&
                               //getWidth() <= peerbounds.getWidth() &&
                               //getHeight() <= peerbounds.getHeight());
                //}

                //if (useback) {
                //    sData = GLXSurfaceData.createData(peer, false);
                //} else {
                    GLXGraphicsConfig gc =
                        (GLXGraphicsConfig)vImg.getGraphicsConfig();
                    ColorModel cm = gc.getColorModel();
                    sData =
                        GLXSurfaceData.createData(gc,
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
        // REMIND: might be too paranoid here...
        return ((gc == null) || (gc == vImg.getGraphicsConfig()));
    }

    public void flush() {
	lostSurface = true;
	GLXSurfaceData oldSD = (GLXSurfaceData)sdAccel;
	sdAccel = null;
	if (oldSD != null) {
	    oldSD.flush();
	}
    }
}
