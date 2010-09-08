/*
 * @(#)GLXVolatileSurfaceManager.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.BufferCapabilities;
import static java.awt.BufferCapabilities.FlipContents.*;
import java.awt.Component;
import java.awt.GraphicsConfiguration;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.X11ComponentPeer;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.BackBufferCapsProvider;
import sun.java2d.SurfaceData;
import static sun.java2d.opengl.OGLContext.OGLContextCaps.*;
import sun.java2d.pipe.hw.ExtendedBufferCapabilities;
import static sun.java2d.pipe.hw.AccelSurface.*;
import static sun.java2d.pipe.hw.ExtendedBufferCapabilities.VSyncType.*;

public class GLXVolatileSurfaceManager extends VolatileSurfaceManager {

    private boolean accelerationEnabled;

    public GLXVolatileSurfaceManager(SunVolatileImage vImg, Object context) {
        super(vImg, context);

        /*
         * We will attempt to accelerate this image only under the
         * following conditions:
         *   - the image is opaque OR
         *   - the image is translucent AND
         *       - the GraphicsConfig supports the FBO extension OR
         *       - the GraphicsConfig has a stored alpha channel
         */
        int transparency = vImg.getTransparency();
        GLXGraphicsConfig gc = (GLXGraphicsConfig)vImg.getGraphicsConfig();
        accelerationEnabled =
            (transparency == Transparency.OPAQUE) ||
            ((transparency == Transparency.TRANSLUCENT) &&
             (gc.isCapPresent(CAPS_EXT_FBOBJECT) ||
              gc.isCapPresent(CAPS_STORED_ALPHA)));
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
            boolean createVSynced = false;
            boolean forceback = false;
            if (context instanceof Boolean) {
                forceback = ((Boolean)context).booleanValue();
                if (forceback && peer instanceof BackBufferCapsProvider) {
                    BackBufferCapsProvider provider =
                        (BackBufferCapsProvider)peer;
                    BufferCapabilities caps = provider.getBackBufferCaps();
                    if (caps instanceof ExtendedBufferCapabilities) {
                        ExtendedBufferCapabilities ebc =
                            (ExtendedBufferCapabilities)caps;
                        if (ebc.getVSync() == VSYNC_ON &&
                            ebc.getFlipContents() == COPIED)
                        {
                            createVSynced = true;
                            forceback = false;
                        }
                    }
                }
            }

            if (forceback) {
                // peer must be non-null in this case
                sData = GLXSurfaceData.createData(peer, vImg, FLIP_BACKBUFFER);
            } else {
                GLXGraphicsConfig gc =
                    (GLXGraphicsConfig)vImg.getGraphicsConfig();
                ColorModel cm = gc.getColorModel(vImg.getTransparency());
                int type = vImg.getForcedAccelSurfaceType();
                // if acceleration type is forced (type != UNDEFINED) then
                // use the forced type, otherwise choose one based on caps
                if (type == OGLSurfaceData.UNDEFINED) {
                    type = gc.isCapPresent(CAPS_EXT_FBOBJECT) ?
                        OGLSurfaceData.FBOBJECT : OGLSurfaceData.PBUFFER;
                }
                if (createVSynced) {
                    sData = GLXSurfaceData.createData(peer, vImg, type);
                } else {
                    sData = GLXSurfaceData.createData(gc,
                                                      vImg.getWidth(),
                                                      vImg.getHeight(),
                                                      cm, vImg, type);
                }
            }
        } catch (NullPointerException ex) {
            sData = null;
        } catch (OutOfMemoryError er) {
            sData = null;
        }

        return sData;
    }

    @Override
    protected boolean isConfigValid(GraphicsConfiguration gc) {
        return ((gc == null) || (gc == vImg.getGraphicsConfig()));
    }

    @Override
    public void initContents() {
        if (vImg.getForcedAccelSurfaceType() != OGLSurfaceData.TEXTURE) {
            super.initContents();
        }
    }
}
