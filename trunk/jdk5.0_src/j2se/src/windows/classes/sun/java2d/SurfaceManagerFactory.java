/*
 * @(#)SurfaceManagerFactory.java	1.3 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.GraphicsConfiguration;
import java.awt.image.BufferedImage;
import sun.awt.WindowsFlags;
import sun.awt.windows.WinCachingSurfaceManager;
import sun.awt.windows.WinVolatileSurfaceManager;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.SurfaceManager;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.opengl.WGLGraphicsConfig;
import sun.java2d.opengl.WGLCachingSurfaceManager;
import sun.java2d.opengl.WGLVolatileSurfaceManager;

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
        } else {
            return new WinCachingSurfaceManager(img);
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
        } else {
            return new WinVolatileSurfaceManager(vImg, context);
        }
    }
}
