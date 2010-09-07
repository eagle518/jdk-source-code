/*
 * @(#)X11GraphicsDevice.java	1.22 04/04/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.DisplayMode;
import java.awt.GraphicsEnvironment;
import java.awt.GraphicsDevice;
import java.awt.GraphicsConfiguration;
import java.awt.Rectangle;
import java.util.HashSet;

import sun.java2d.opengl.GLXGraphicsConfig;

/**
 * This is an implementation of a GraphicsDevice object for a single
 * X11 screen.
 *
 * @see GraphicsEnvironment
 * @see GraphicsConfiguration
 * @version 10 Feb 1997
 */
public class X11GraphicsDevice extends GraphicsDevice {
    int screen;
    private static DisplayMode displayMode;

    public X11GraphicsDevice(int screennum) {
	this.screen = screennum;
    }

    /*
     * Initialize JNI field and method IDs for fields that may be
     * accessed from C.
     */
    private static native void initIDs();

    static {
        if (!GraphicsEnvironment.isHeadless()) {
            initIDs();
        }
    }

    /**
     * Returns the X11 screen of the device.
     */
    public int getScreen() {
        return screen;
    }

    /**
     * Returns the X11 Display of this device.
     * This method is also in MDrawingSurfaceInfo but need it here
     * to be able to allow a GraphicsConfigTemplate to get the Display.
     */
    public native long getDisplay();

    /**
     * Returns the type of the graphics device.
     * @see #TYPE_RASTER_SCREEN
     * @see #TYPE_PRINTER
     * @see #TYPE_IMAGE_BUFFER
     */
    public int getType() {
	return TYPE_RASTER_SCREEN;
    }

    /**
     * Returns the identification string associated with this graphics
     * device.
     */
    public String getIDstring() {
	return ":0."+screen;
    }


    GraphicsConfiguration[] configs;
    GraphicsConfiguration defaultConfig;
    HashSet doubleBufferVisuals;

    /**
     * Returns all of the graphics
     * configurations associated with this graphics device.
     */
    public GraphicsConfiguration[] getConfigurations() {
        GraphicsConfiguration[] ret = configs;
        if (ret == null) {
            int i = 1;  // Index 0 is always the default config
            int num = getNumConfigs(screen);
            ret = new GraphicsConfiguration[num];
            if (defaultConfig == null) {
                ret [0] = getDefaultConfiguration();
            }
            else {
                ret [0] = defaultConfig;
            }

            boolean glxSupported = GLXGraphicsConfig.isGLXAvailable();
            boolean dbeSupported = isDBESupported();
            if (dbeSupported && doubleBufferVisuals == null) {
                doubleBufferVisuals = new HashSet();
                getDoubleBufferVisuals(screen);
            }
            for ( ; i < num; i++) {
                int visNum = getConfigVisualId(i, screen);
                int depth = getConfigDepth (i, screen);
                if (glxSupported) {
                    ret[i] = GLXGraphicsConfig.getConfig(this, visNum);
                }
                if (ret[i] == null) {
                    boolean doubleBuffer =
                        (dbeSupported &&
                         doubleBufferVisuals.contains(new Integer(visNum)));
                    ret[i] = X11GraphicsConfig.getConfig(this, visNum, depth,
                            getConfigColormap(i, screen),
                            doubleBuffer);
                }
            }
            configs = ret;
        }
        return ret;
    }

    /*
     * Returns the number of X11 visuals representable as an
     * X11GraphicsConfig object.
     */
    public native int getNumConfigs(int screen);

    /*
     * Returns the visualid for the given index of graphics configurations.
     */
    public native int getConfigVisualId (int index, int screen);
    /*
     * Returns the depth for the given index of graphics configurations.
     */
    public native int getConfigDepth (int index, int screen);

    /*
     * Returns the colormap for the given index of graphics configurations.
     */
    public native int getConfigColormap (int index, int screen);


    // Whether or not double-buffering extension is supported
    public static native boolean isDBESupported();
    // Callback for adding a new double buffer visual into our set
    private void addDoubleBufferVisual(int visNum) {
        doubleBufferVisuals.add(new Integer(visNum));
    }
    // Enumerates all visuals that support double buffering
    private native void getDoubleBufferVisuals(int screen);
    
    /**
     * Returns the default graphics configuration
     * associated with this graphics device.
     */
    public GraphicsConfiguration getDefaultConfiguration() {
	if (defaultConfig == null) {
            int visNum = getConfigVisualId(0, screen);
           int depth = getConfigDepth(0, screen);
            if (GLXGraphicsConfig.isGLXAvailable()) {
                defaultConfig = GLXGraphicsConfig.getConfig(this, visNum);
                if (GLXGraphicsConfig.isGLXVerbose()) {
                    if (defaultConfig != null) {
                        System.out.print("OpenGL pipeline enabled");
                    } else {
                        System.out.print("Could not enable OpenGL pipeline");
                    }
                    System.out.println(" for default config on screen " +
                                       screen);
                }
            }
            if (defaultConfig == null) {
                boolean doubleBuffer = false;
                if (isDBESupported() && doubleBufferVisuals == null) {
                    doubleBufferVisuals = new HashSet();
                    getDoubleBufferVisuals(screen);
                    doubleBuffer =
                        doubleBufferVisuals.contains(new Integer(visNum));
                }
                defaultConfig = X11GraphicsConfig.getConfig(this, visNum,
                                                            depth, getConfigColormap(0, screen),
                                                            doubleBuffer);
            }
	}

	return defaultConfig;
    }

    public DisplayMode getDisplayMode() {
        if (displayMode == null) {
            GraphicsConfiguration gc = getDefaultConfiguration();
            Rectangle r = gc.getBounds();
            displayMode = new DisplayMode(r.width, r.height,
                DisplayMode.BIT_DEPTH_MULTI, DisplayMode.REFRESH_RATE_UNKNOWN);
        }
        return displayMode;
    }

    public String toString() {
	return ("X11GraphicsDevice[screen="+screen+"]");
    }

}
