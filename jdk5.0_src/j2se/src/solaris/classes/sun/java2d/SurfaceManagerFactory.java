/*
 * @(#)SurfaceManagerFactory.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.GraphicsConfiguration;
import java.awt.image.BufferedImage;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11SurfaceData;
import sun.awt.motif.X11CachingSurfaceManager;
import sun.awt.motif.X11VolatileSurfaceManager;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.SurfaceManager;
import sun.awt.image.VolatileSurfaceManager;
import sun.java2d.opengl.GLXGraphicsConfig;
import sun.java2d.opengl.GLXSurfaceData;
import sun.java2d.opengl.GLXVolatileSurfaceManager;

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
     * For Unix platforms, this method returns a CachingSurfaceManager that
     * is capable of managing/caching both X11 and GLX surfaces.
     */
    public static SurfaceManager createCachingManager(BufferedImage img) {
        return new X11CachingSurfaceManager(img);
    }

    /**
     * Creates a new instance of a VolatileSurfaceManager given any
     * arbitrary SunVolatileImage.  An optional context Object can be supplied
     * as a way for the caller to pass pipeline-specific context data to
     * the VolatileSurfaceManager (such as a backbuffer handle, for example).
     *
     * For Unix platforms, this method returns either an X11- or a GLX-
     * specific VolatileSurfaceManager based on the GraphicsConfiguration
     * under which the SunVolatileImage was created.
     */
    public static VolatileSurfaceManager
        createVolatileManager(SunVolatileImage vImg,
                              Object context)
    {
        GraphicsConfiguration gc = vImg.getGraphicsConfig();
        if (gc instanceof GLXGraphicsConfig) {
            return new GLXVolatileSurfaceManager(vImg, context);
        } else {
            return new X11VolatileSurfaceManager(vImg, context);
        }
    }
}
