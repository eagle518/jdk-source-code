/*
 * @(#)WWindowPeer.java	1.46 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.util.Vector;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.Win32GraphicsDevice;
import sun.awt.Win32GraphicsConfig;
import java.lang.ref.WeakReference;
import sun.awt.DebugHelper;
import sun.awt.DisplayChangedListener;
import sun.awt.SunToolkit;

public class WWindowPeer extends WPanelPeer implements WindowPeer {
    private static final DebugHelper dbg =
        DebugHelper.create(WWindowPeer.class);

    /**
     * Initialize JNI field IDs
     */
    private static native void initIDs();
    static {
        initIDs();
    }
    
    protected boolean focusableWindow; // value queried from native code

    // WComponentPeer overrides

    protected void disposeImpl() {
        // Remove ourself from the Map of DisplayChangeListeners
        GraphicsConfiguration gc = getGraphicsConfiguration();
        ((Win32GraphicsDevice)gc.getDevice()).removeDisplayChangedListener(this);
        allWindows.removeElement(this);
        super.disposeImpl();
    }

    // WindowPeer implementation

    public void toFront() {
        focusableWindow = ((Window)target).isFocusableWindow();
        _toFront();
    }
    native void _toFront();
    public native void toBack();

    public void updateAlwaysOnTop() {
        setAlwaysOnTop(((Window)target).isAlwaysOnTop());
    }
    native void setAlwaysOnTop(boolean value);

    // FramePeer & DialogPeer partial shared implementation

    public void setTitle(String title) {
        // allow a null title to pass as an empty string.
        if (title == null) {
            title = new String("");
        }
        _setTitle(title);
    }
    native void _setTitle(String title);

    public void setResizable(boolean resizable) {
        _setResizable(resizable);
    }
    public native void _setResizable(boolean resizable);

    // Toolkit & peer internals

    static Vector allWindows = new Vector();  //!CQ for anchoring windows, frames, dialogs

    WWindowPeer(Window target) {
        super(target);
    }

    void initialize() {
        super.initialize();

        updateInsets(insets_);
        allWindows.addElement(this);

        Font f = ((Window)target).getFont();
        if (f == null) {
            f = defaultFont;
            ((Window)target).setFont(f);
            setFont(f);
        }
        // Express our interest in display changes
        GraphicsConfiguration gc = getGraphicsConfiguration();
        ((Win32GraphicsDevice)gc.getDevice()).addDisplayChangedListener(this);
    }

    native void createAwtWindow(WComponentPeer parent);
    void create(WComponentPeer parent) {
        createAwtWindow(parent);
    }
        
    public void show() {
        focusableWindow = ((Window)target).isFocusableWindow();
        super.show();
        if (((Window)target).isAlwaysOnTop()) {
            setAlwaysOnTop(true);
        }
    }

    // Synchronize the insets members (here & in helper) with actual window
    // state.
    native void updateInsets(Insets i);

    private native Component getContainerElement(Container c, int i);

    static native int getSysMinWidth();
    static native int getSysMinHeight();

    synchronized native void reshapeFrame(int x, int y, int width, int height);
    public boolean requestWindowFocus() {
        // Win32 window doesn't need this
        return false;
    }    
/*
 * ----DISPLAY CHANGE SUPPORT----
 */

    /*
     * Called from native code when we have been dragged onto another screen.
     */
    void draggedToNewScreen() {
        SunToolkit.executeOnEventHandlerThread((Component)target,new Runnable() 
        {
            public void run() {
                displayChanged();
            }
        });
    }
	

    /*
     * Called from WCanvasPeer.displayChanged().
     * Override to do nothing - Window and WWindowPeer GC must never be set to
     * null!
     */
    void clearLocalGC() {}

    /*
     * Called from WCanvasPeer.displayChanged().
     * Reset the graphicsConfiguration member of our target Component.
     * Component.resetGC() is a package-private method, so we have to call it
     * through JNI.
     */
    native void resetTargetGC();

    /*
     * From the DisplayChangedListener interface
     *
     * This method handles a display change - either when the display settings
     * are changed, or when the window has been dragged onto a different
     * display.
     */
    public void displayChanged() {
        int scrn = getScreenImOn();

        // get current GD
        Win32GraphicsDevice oldDev = (Win32GraphicsDevice)winGraphicsConfig
                                     .getDevice();

        // get new GD
        Win32GraphicsDevice newDev = (Win32GraphicsDevice)GraphicsEnvironment
            .getLocalGraphicsEnvironment()
            .getScreenDevices()[scrn];

        // Set winGraphicsConfig to the default GC for the monitor this Window
        // is now mostly on.
        winGraphicsConfig = (Win32GraphicsConfig)newDev
                            .getDefaultConfiguration();
        if (dbg.on) {
            dbg.assertion(winGraphicsConfig != null);
        }

        // if on a different display, take off old GD and put on new GD
        if (oldDev != newDev) {
            oldDev.removeDisplayChangedListener(this);
            newDev.addDisplayChangedListener(this);
        }
        super.displayChanged();
    }

    private native int getScreenImOn();

/*
 * ----END DISPLAY CHANGE SUPPORT----
 */
}
