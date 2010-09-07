/*
 * @(#)Win32GraphicsDevice.java	1.46 04/04/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.AWTPermission;
import java.awt.GraphicsDevice;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.image.ColorModel;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Vector;
import java.awt.peer.ComponentPeer;
import java.awt.peer.WindowPeer;
import sun.awt.windows.WComponentPeer;
import sun.awt.SunDisplayChanger;
import sun.awt.DisplayChangedListener;
import sun.awt.windows.WWindowPeer;
import sun.java2d.InvalidPipeException;
import sun.java2d.opengl.WGLGraphicsConfig;

/**
 * This is an implementation of a GraphicsDevice object for a single
 * Win32 screen.
 *
 * @see GraphicsEnvironment
 * @see GraphicsConfiguration
 * @version 10 Feb 1997
 */
public class Win32GraphicsDevice extends GraphicsDevice implements 
 DisplayChangedListener {
    int screen;
    ColorModel dynamicColorModel;   // updated with dev changes
    ColorModel colorModel;          // static for device
    GraphicsConfiguration[] configs;
    GraphicsConfiguration defaultConfig;
    boolean offscreenAccelerationEnabled = true;

    // keep track of top-level windows on this display
    private SunDisplayChanger topLevels = new SunDisplayChanger();
    private static boolean pfDisabled;
    private static AWTPermission fullScreenExclusivePermission;
    private Rectangle ownerWindowedModeBounds = null;

    static {

        // 4455041 - Even when ddraw is disabled, ddraw.dll is loaded when
        // pixel format calls are made.  This causes problems when a Java app
        // is run as an NT service.  To prevent the loading of ddraw.dll
        // completely, sun.awt.nopixfmt should be set as well.  Apps which use
        // OpenGL w/ Java probably don't want to set this.
        String nopixfmt = (String)java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.awt.nopixfmt"));
        pfDisabled = (nopixfmt != null);
	initIDs();
    }    

    private static native void initIDs();

    /**
     * Acceleration can be disabled due to capabilities of the display
     * device discovered during ddraw initialization.  This is not the
     * same as isDDEnabledOnDevice(), which returns false when ddraw
     * was disabled by the user or had problems initializing.
     */
    public boolean isOffscreenAccelerationEnabled() {
	return offscreenAccelerationEnabled;
    }

    native void initDevice(int screen);

    public Win32GraphicsDevice(int screennum) {
        this.screen = screennum;
	initDevice(screennum);
    }

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
     * Returns the Win32 screen of the device.
     */
    public int getScreen() {
        return screen;
    }
    
    /**
     * Returns the identification string associated with this graphics
     * device.
     */
    public String getIDstring() {
	return "\\Display"+screen;
    }


    /**
     * Returns all of the graphics
     * configurations associated with this graphics device.
     */
    public GraphicsConfiguration[] getConfigurations() {
        if (configs==null) {
            if (WindowsFlags.isOGLEnabled() && isDefaultDevice()) {
                defaultConfig = getDefaultConfiguration();
                if (defaultConfig != null) {
                    configs = new GraphicsConfiguration[1];
                    configs[0] = defaultConfig;
                    return configs;
                }
            }

            int max = getMaxConfigs(screen);
            int defaultPixID = getDefaultPixID(screen);
            Vector v = new Vector( max );
            if (defaultPixID == 0) {
                // Workaround for failing GDI calls, or if DirectDraw
                // is disabled
                defaultConfig = Win32GraphicsConfig.getConfig(this,
                                                              defaultPixID);
                v.addElement(defaultConfig);
            }
            else {
                for (int i = 1; i <= max; i++) {
                    if (isPixFmtSupported(i, screen)) {
                        if (i == defaultPixID) {
                            defaultConfig = Win32GraphicsConfig.getConfig(
                             this, i);
                            v.addElement(defaultConfig);
                        }
                        else {
                            v.addElement(Win32GraphicsConfig.getConfig(
                             this, i));
                        }
                    }
                }
            }
            configs = new GraphicsConfiguration[v.size()];
            v.copyInto(configs);
        }
        return configs;
    }

    /**
     * Returns the maximum number of graphics configurations available, or 1
     * if PixelFormat calls fail or are disabled.
     * This number is less than or equal to the number of graphics
     * configurations supported.
     */
    protected int getMaxConfigs(int screen) {
        if (pfDisabled) {
            return 1;
        } else {
            return getMaxConfigsImpl(screen);
        }
    }
  
    private native int getMaxConfigsImpl(int screen);

    /**
     * Returns whether or not the PixelFormat indicated by index is 
     * supported.  Supported PixelFormats support drawing to a Window
     * (PFD_DRAW_TO_WINDOW), support GDI (PFD_SUPPORT_GDI), and in the
     * case of an 8-bit format (cColorBits <= 8) uses indexed colors 
     * (iPixelType == PFD_TYPE_COLORINDEX).
     * We use the index 0 to indicate that PixelFormat calls don't work, or
     * are disabled.  Do not call this function with an index of 0.
     * @param index a PixelFormat index
     */
    protected native boolean isPixFmtSupported(int index, int screen);

    /**
     * Returns the PixelFormatID of the default graphics configuration
     * associated with this graphics device, or 0 if PixelFormats calls fail or
     * are disabled.
     */
    protected int getDefaultPixID(int screen) {
        if (pfDisabled) {
            return 0;
        } else {
            return getDefaultPixIDImpl(screen);
        }
    }

    /**
     * Returns the default PixelFormat ID from GDI.  Do not call if PixelFormats
     * are disabled.
     */
    private native int getDefaultPixIDImpl(int screen);
    
    /**
     * Returns the default graphics configuration
     * associated with this graphics device.
     */
    public GraphicsConfiguration getDefaultConfiguration() {
        if (defaultConfig == null) {
            // first try to create a WGLGraphicsConfig if OGL is enabled
            // REMIND: the WGL code does not yet work properly in multimon
            // situations, so we will fallback on GDI if we are not on the
            // default device...
            if (WindowsFlags.isOGLEnabled() && isDefaultDevice()) {
                int defPixID = WGLGraphicsConfig.getDefaultPixFmt(screen);
                defaultConfig = WGLGraphicsConfig.getConfig(this, defPixID);
                if (WindowsFlags.isOGLVerbose()) {
                    if (defaultConfig != null) {
                        System.out.print("OpenGL pipeline enabled");
                    } else {
                        System.out.print("Could not enable OpenGL pipeline");
                    }
                    System.out.println(" for default config on screen " +
                                       screen);
                }
            }

            // Fix for 4669614.  Most apps are not concerned with PixelFormats,
            // yet we ALWAYS used them for determining ColorModels and such.
            // By passing in 0 as the PixelFormatID here, we signal that 
            // PixelFormats should not be used, thus avoid loading the opengl
            // library.  Apps concerned with PixelFormats can still use
            // GraphicsConfiguration.getConfigurations().
            // Note that calling native pixel format functions tends to cause
            // problems between those functions (which are OpenGL-related)
            // and our use of DirectX.  For example, some Matrox boards will
            // crash or hang calling these functions when any app is running
            // in DirectX fullscreen mode.  So avoiding these calls unless
            // absolutely necessary is preferable.
            if (defaultConfig == null) {
                defaultConfig = Win32GraphicsConfig.getConfig(this, 0);
            }
        }
        return defaultConfig;
    }

    public String toString() {
        return ("Win32GraphicsDevice[screen=" + screen + "]");
    }

    /**
     * Returns true if this is the default GraphicsDevice for the
     * GraphicsEnvironment.
     */
    private boolean isDefaultDevice() {
        return (this ==
                GraphicsEnvironment.
                    getLocalGraphicsEnvironment().getDefaultScreenDevice());
    }

    private native boolean isDDEnabledOnDeviceNative(int screen);
    
    public boolean isDDEnabledOnDevice() {
	return (WindowsFlags.isDDEnabled() && 
		isDDEnabledOnDeviceNative(screen));
    }
    
    private native boolean isD3DEnabledOnDeviceNative(int screen);
    
    public boolean isD3DEnabledOnDevice() {
	return (WindowsFlags.isD3DEnabled() && 
		isD3DEnabledOnDeviceNative(screen));
    }
    
    public boolean isFullScreenSupported() {
	// Currently non-ddraw fullscreen mode is supported only on the first
	// screen, so return true iff dd is enabled OR we are on screen 0
        return (screen == 0 || isDDEnabledOnDevice());
    }
    
    /**
     * Return the owning Frame for a given Window.  Used in setFSWindow below
     * to set the properties of the owning Frame when a Window goes
     * into fullscreen mode.
     */
    private Frame getToplevelOwner(Window w) {
	Window owner = w;
	while (owner != null) {
	    owner = owner.getOwner();
	    if (owner instanceof Frame) {
		return (Frame) owner;
	    }
	}
        // Should not get here, but return something intelligent just in case
        return null;
    }
    
    public synchronized void setFullScreenWindow(Window w) {
        Window old = getFullScreenWindow();
        if (w == old) {
            return;
        }
	boolean ddEnabledOnDevice = isDDEnabledOnDevice();
        if (ddEnabledOnDevice && old != null) {
            // Enter windowed mode.
            WWindowPeer peer = (WWindowPeer)old.getPeer();
            if (peer != null) {
                synchronized(peer) {
                    peer.destroyBuffers();
                    exitFullScreenExclusive(screen, peer);
                }
            }
            /**
             * Bug 4933099: There is some funny-business to deal with when this 
             * method is called with a Window instead of a Frame.  See 4836744 
             * for more information on this.  One side-effect of our workaround
             * for the problem is that the owning Frame of a Window may end
             * up getting resized during the fullscreen process.  When we
             * return from fullscreen mode, we should resize the Frame to
             * its original size (just like the Window is being resized
             * to its original size in GraphicsDevice).
             */
            if (!(old instanceof Frame)) {
                Frame owner = getToplevelOwner(old);
                if (owner != null && ownerWindowedModeBounds != null) {
                    owner.setBounds(ownerWindowedModeBounds);
                }
		ownerWindowedModeBounds = null;
            }
        }    
        super.setFullScreenWindow(w);
        if (ddEnabledOnDevice && w != null) {
            // Bug 4933099
            if (!(w instanceof Frame)) {
                Frame owner = getToplevelOwner(w);
                if (owner != null) {
                    ownerWindowedModeBounds = owner.getBounds();
                    // These will get set for the native window in 
                    // any case.  Set them here so that resetting them
                    // later actually does the right thing
                    owner.setBounds(w.getBounds());
                }
            }
            SecurityManager security = System.getSecurityManager();
            if (security != null) {
                if (fullScreenExclusivePermission == null) {
                    fullScreenExclusivePermission =
                        new AWTPermission("fullScreenExclusive");
                }
                security.checkPermission(fullScreenExclusivePermission);
            }
            // Enter full screen exclusive mode.
            WWindowPeer peer = (WWindowPeer)w.getPeer();
            synchronized(peer) {
                enterFullScreenExclusive(screen, peer);
                // Note: removed replaceSurfaceData() call because
                // changing the window size or making it visible
                // will cause this anyway, and both of these events happen
                // as part of switching into fullscreen mode.
            }
        }
    }
    
    // Entering and exiting full-screen mode are done within a
    // tree-lock and should never lock on any resources which are
    // required by other threads which may have them and may require
    // the tree-lock.
    // These functions should only be called if DirectDraw is enabled.
    private native void enterFullScreenExclusive(int screen, WindowPeer w);
    private native void exitFullScreenExclusive(int screen, WindowPeer w);
    
    public boolean isDisplayChangeSupported() {
        return (isFullScreenSupported() && getFullScreenWindow() != null);
    }

    public synchronized void setDisplayMode(DisplayMode dm) {
        if (!isDisplayChangeSupported()) {
            super.setDisplayMode(dm);
            return;
        }
        if (dm == null || !isDisplayModeAvailable(dm)) {
            throw new IllegalArgumentException(
                "Invalid display mode");
        }
        Window w = getFullScreenWindow();
        if (w != null) {
            WWindowPeer peer = (WWindowPeer)w.getPeer();
	    configDisplayMode(screen, peer, dm.getWidth(), dm.getHeight(),
		dm.getBitDepth(), dm.getRefreshRate());
	    // Note: no call to replaceSurfaceData is required here since
	    // replacement will be caused by an upcoming display change event
        } else {
	    throw new IllegalStateException("Must be in fullscreen mode " +
	    				    "in order to set display mode");
	}
    }

    private native DisplayMode getCurrentDisplayMode(int screen);
    private native void configDisplayMode(int screen, WindowPeer w, int width,
    					  int height, int bitDepth,
    					  int refreshRate);
    private native void enumDisplayModes(int screen, ArrayList modes);
    // This function is only available if DirectDraw is enabled, otherwise we
    // have to do the work the hard way (enumerating all of the display modes
    // and checking each one)
    private native boolean isDisplayModeAvailable(int screen, int width, int height,
        int bitDepth, int refreshRate);

    public synchronized DisplayMode getDisplayMode() {
        DisplayMode res = getCurrentDisplayMode(screen);
        return res;
    }

    public synchronized DisplayMode[] getDisplayModes() {
        ArrayList modes = new ArrayList();
        enumDisplayModes(screen, modes);
        int listSize = modes.size();
        DisplayMode[] retArray = new DisplayMode[listSize];
        for (int i = 0; i < listSize; i++) {
            retArray[i] = (DisplayMode)modes.get(i);
        }
        return retArray;
    }
    
    public synchronized boolean isDisplayModeAvailable(DisplayMode dm) {
        if (!isDisplayChangeSupported()) {
            return false;
        }
        if (isDDEnabledOnDevice()) {
            return isDisplayModeAvailable(screen, dm.getWidth(), dm.getHeight(),
                dm.getBitDepth(), dm.getRefreshRate());
        } else {
            // The function isDisplayModeAvailable is only available if
            // DirectDraw is enabled, otherwise we have to do the work the
            // hard way (enumerating all of the display modes
            // and checking each one)
            DisplayMode[] modes = getDisplayModes();
            for (int i = 0; i < modes.length; i++) {
                if (dm.equals(modes[i])) {
                    return true;
                }
            }
            return false;
        }
    }

    /*
     * From the DisplayChangeListener interface.
     * Called from Win32GraphicsEnvironment when the display settings have
     * changed.
     */
    public void displayChanged() {
	dynamicColorModel = null;
        defaultConfig = null;
        configs = null;    
        // pass on to all top-level windows on this display
        topLevels.notifyListeners();
    }

    /**
     * Part of the DisplayChangedListener interface: devices
     * do not need to react to this event
     */
    public void paletteChanged() {
    }

    /*
     * Add a DisplayChangeListener to be notified when the display settings
     * are changed.  Typically, only top-level containers need to be added
     * to Win32GraphicsDevice.
     */
    public void addDisplayChangedListener(DisplayChangedListener client) {
        topLevels.add(client);
    }

    /*
     * Remove a DisplayChangeListener from this Win32GraphicsDevice
     */
     public void removeDisplayChangedListener(DisplayChangedListener client) {
        topLevels.remove(client);
    }

    /**
     * Creates and returns the color model associated with this device
     */
    private native ColorModel makeColorModel (int screen, 
					      boolean dynamic);

    /**
     * Returns a dynamic ColorModel which is updated when there
     * are any changes (e.g., palette changes) in the device
     */
    public ColorModel getDynamicColorModel() {
	if (dynamicColorModel == null) {
	    dynamicColorModel = makeColorModel(screen, true);
	}
	return dynamicColorModel;
    }

    /**
     * Returns the non-dynamic ColorModel associated with this device
     */
    public ColorModel getColorModel() {
        if (colorModel == null)  {
	    colorModel = makeColorModel(screen, false);
	}
	return colorModel;
    }

    private native int getDeviceMemoryNative(int screen);

    /**
     * Returns number of bytes available in VRAM on this device.
     */
    public int getAvailableAcceleratedMemory() {
	return getDeviceMemoryNative(screen);
    }
}

