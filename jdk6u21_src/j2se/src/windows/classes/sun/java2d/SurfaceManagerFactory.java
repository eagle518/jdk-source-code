/*
 * @(#)SurfaceManagerFactory.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.image.BufferedImage;
import sun.awt.image.BufImgVolatileSurfaceManager;
import sun.awt.image.CachingSurfaceManager;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.SurfaceManager;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.d3d.D3DCachingSurfaceManager;
import sun.java2d.d3d.D3DGraphicsConfig;
import sun.java2d.d3d.D3DGraphicsDevice;
import sun.java2d.d3d.D3DVolatileSurfaceManager;
import sun.java2d.loops.CompositeType;
import sun.java2d.opengl.WGLGraphicsConfig;
import sun.java2d.opengl.WGLCachingSurfaceManager;
import sun.java2d.opengl.WGLVolatileSurfaceManager;
import sun.java2d.windows.WindowsFlags;

/**
 * This is a factory class with static methods for creating a
 * platform-specific instance of a particular SurfaceManager.  Each platform
 * (Windows, Unix, etc.) has its own specialized SurfaceManagerFactory.
 */
public class SurfaceManagerFactory {

    /**
     * Creates a new instance of a CachingSurfaceManager given any
     * arbitrary BufferedImage.
     *
     * For Windows platforms, this method returns a CachingSurfaceManager that
     * is capable of managing/caching various Windows surfaces (such as
     * DirectDraw surfaces or Direct3D textures).  If OGL is enabled, a
     * special WGLCachingSurfaceManager is returned that is capable of
     * managing only OGL textures.
     */
    public static SurfaceManager createCachingManager(BufferedImage img) {
        if (WindowsFlags.isOGLEnabled()) {
            return new WGLCachingSurfaceManager(img);
        } else if (D3DGraphicsDevice.isD3DAvailable()) {
            return new D3DCachingSurfaceManager(img);
        } else {
            return new CachingSurfaceManager(img) {
                protected SurfaceData
                    createAccelSurface(GraphicsConfiguration gc,
                                       int width, int height)
                {
                    return null;
                }
                protected boolean isDestSurfaceAccelerated(SurfaceData destSD) {
                    return false;
                }
                protected boolean isOperationSupported(SurfaceData dstData,
                        CompositeType comp, Color bgColor, boolean scale)
                {
                    return false;
                }
            };
        }
    }

    /**
     * Creates a new instance of a VolatileSurfaceManager given any
     * arbitrary SunVolatileImage.  An optional context Object can be supplied
     * as a way for the caller to pass pipeline-specific context data to
     * the VolatileSurfaceManager (such as a backbuffer handle, for example).
     *
     * For Windows platforms, this method returns a Windows-specific
     * VolatileSurfaceManager.
     */
    public static VolatileSurfaceManager
        createVolatileManager(SunVolatileImage vImg,
                              Object context)
    {
        GraphicsConfiguration gc = vImg.getGraphicsConfig();
        if (gc instanceof WGLGraphicsConfig) {
            return new WGLVolatileSurfaceManager(vImg, context);
        } else if (gc instanceof D3DGraphicsConfig) {
            return new D3DVolatileSurfaceManager(vImg, context);
        } else {
            return new BufImgVolatileSurfaceManager(vImg, context);
        }
    }
}
