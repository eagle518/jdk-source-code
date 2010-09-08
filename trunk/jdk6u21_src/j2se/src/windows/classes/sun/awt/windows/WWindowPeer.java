/*
 * @(#)WWindowPeer.java	1.80 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.peer.*;

import java.beans.*;

import java.lang.reflect.*;

import java.util.*;
import java.util.List;
import java.util.logging.*;

import sun.awt.*;

public class WWindowPeer extends WPanelPeer implements WindowPeer {
    private static final DebugHelper dbg =
        DebugHelper.create(WWindowPeer.class);

    private static final Logger log = Logger.getLogger("sun.awt.windows.WWindowPeer");

    // we can't use WDialogPeer as blocker may be an instance of WPrintDialogPeer that
    // extends WWindowPeer, not WDialogPeer
    private WWindowPeer modalBlocker = null;

    /*
     * A key used for storing a list of active windows in AppContext. The value
     * is a list of windows, sorted by the time of activation: later a window is
     * activated, greater its index is in the list.
     */
    private final static StringBuffer ACTIVE_WINDOWS_KEY =
        new StringBuffer("active_windows_list");

    /*
     * Listener for 'activeWindow' KFM property changes. It is added to each
     * AppContext KFM. See ActiveWindowListener inner class below.
     */
    private static PropertyChangeListener activeWindowListener =
        new ActiveWindowListener();

    /*
     * The object is a listener for the AppContext.GUI_DISPOSED property.
     */
    private final static PropertyChangeListener guiDisposedListener =
        new GuiDisposedListener();


    /**
     * Initialize JNI field IDs
     */
    private static native void initIDs();
    static {
        initIDs();
    }
    
    // WComponentPeer overrides

    protected void disposeImpl() {
        AppContext appContext = SunToolkit.targetToAppContext(target);
        synchronized (appContext) {
            List<WWindowPeer> l = (List<WWindowPeer>)appContext.get(ACTIVE_WINDOWS_KEY);
            if (l != null) {
                l.remove(this);
            }
        }
        // Remove ourself from the Map of DisplayChangeListeners
        GraphicsConfiguration gc = getGraphicsConfiguration();
        ((Win32GraphicsDevice)gc.getDevice()).removeDisplayChangedListener(this);
        synchronized (getStateLock()) {
            TranslucentWindowPainter currentPainter = painter;
            if (currentPainter != null) {
                currentPainter.flush();
                // don't set the current one to null here; reduces the chances of
                // MT issues (like NPEs)
            }
        }
        super.disposeImpl();
    }

    // WindowPeer implementation

    public void toFront() {
        updateFocusableWindowState();
        _toFront();
    }
    native void _toFront();
    public native void toBack();

    public native void setAlwaysOnTopNative(boolean value);
    public void setAlwaysOnTop(boolean value) {
        if ((value && ((Window)target).isVisible()) || !value) {
            setAlwaysOnTopNative(value);
        }
    }

    public void updateFocusableWindowState() {
        setFocusableWindow(((Window)target).isFocusableWindow());
    }
    native void setFocusableWindow(boolean value);

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

    WWindowPeer(Window target) {
        super(target);
    }

    void initialize() {
        super.initialize();

        updateInsets(insets_);

        Font f = ((Window)target).getFont();
        if (f == null) {
            f = defaultFont;
            ((Window)target).setFont(f);
            setFont(f);
        }
        // Express our interest in display changes
        GraphicsConfiguration gc = getGraphicsConfiguration();
        ((Win32GraphicsDevice)gc.getDevice()).addDisplayChangedListener(this);

        initActiveWindowsTracking((Window)target);

        updateIconImages();
    }

    native void createAwtWindow(WComponentPeer parent);
    void create(WComponentPeer parent) {
        createAwtWindow(parent);
    }

    protected boolean visible = false;

    // should be overriden in WDialogPeer
    protected void realShow() {
        super.show();
        visible = true;
    }

    public void show() {
        updateFocusableWindowState();

        boolean alwaysOnTop = ((Window)target).isAlwaysOnTop();

        // Fix for 4868278.
        // If we create a window with a specific GraphicsConfig, and then move it with
        // setLocation() or setBounds() to another one before its peer has been created,
        // then calling Window.getGraphicsConfig() returns wrong config. That may lead
        // to some problems like wrong-placed tooltips. It is caused by calling
        // super.displayChanged() in WWindowPeer.displayChanged() regardless of whether
        // GraphicsDevice was really changed, or not. So we need to track it here.
        updateGC();
        resetTargetGC();

        realShow();
        updateMinimumSize();

        if (((Window)target).isAlwaysOnTopSupported() && alwaysOnTop) {
            setAlwaysOnTop(alwaysOnTop);
        }
        updateWindow(true);
    }

    public void hide() {
        super.hide();
        visible = false;
    }

    // Synchronize the insets members (here & in helper) with actual window
    // state.
    native void updateInsets(Insets i);

    static native int getSysMinWidth();
    static native int getSysMinHeight();
    static native int getSysIconWidth();
    static native int getSysIconHeight();
    static native int getSysSmIconWidth();
    static native int getSysSmIconHeight();
    /**
     * Creates native icon from specified raster data and updates
     * icon for window and all descendant windows that inherit icon.
     * Raster data should be passed in the ARGB form.
     * Note that raster data format was changed to provide support
     * for XP icons with alpha-channel
     */
    native void setIconImagesData(int[] iconRaster, int w, int h, 
                                  int[] smallIconRaster, int smw, int smh);

    synchronized native void reshapeFrame(int x, int y, int width, int height);
    public boolean requestWindowFocus() {
        // Win32 window doesn't need this
        return false;
    }

    public boolean focusAllowedFor() {
        Window target = (Window)this.target;
        if (!target.isVisible() ||
            !target.isEnabled() ||
            !target.isFocusable())
        {
            return false;
        }

        if (isModalBlocked()) {
            return false;
        }

        return true;
    }

    public void updateMinimumSize() {
        Dimension minimumSize = null;
        if (((Component)target).isMinimumSizeSet()) {
            minimumSize = ((Component)target).getMinimumSize();
        }
        if (minimumSize != null) {
            int msw = getSysMinWidth();
            int msh = getSysMinHeight();
            int w = (minimumSize.width >= msw) ? minimumSize.width : msw;
            int h = (minimumSize.height >= msh) ? minimumSize.height : msh;
            setMinSize(w, h);
        } else {
            setMinSize(0, 0);
        }
    }

    public void updateIconImages() {
        java.util.List<Image> imageList = ((Window)target).getIconImages();
        if (imageList == null || imageList.size() == 0) {
            setIconImagesData(null, 0, 0, null, 0, 0);
        } else {
            int w = getSysIconWidth();
            int h = getSysIconHeight();
            int smw = getSysSmIconWidth();
            int smh = getSysSmIconHeight();
            DataBufferInt iconData = SunToolkit.getScaledIconData(imageList,
                                                                  w, h);
            DataBufferInt iconSmData = SunToolkit.getScaledIconData(imageList,
                                                                    smw, smh);
            if (iconData != null && iconSmData != null) {
                setIconImagesData(iconData.getData(), w, h, 
                                  iconSmData.getData(), smw, smh);
            } else {
                setIconImagesData(null, 0, 0, null, 0, 0);
            }
        }
    }

    native void setMinSize(int width, int height);

/*
 * ---- MODALITY SUPPORT ----
 */

    /**
     * Some modality-related code here because WFileDialogPeer, WPrintDialogPeer and
     *   WPageDialogPeer are descendants of WWindowPeer, not WDialogPeer
     */

    public boolean isModalBlocked() {
        return modalBlocker != null;
    }

    public void setModalBlocked(Dialog dialog, boolean blocked) {
        synchronized (((Component)getTarget()).getTreeLock()) // State lock should always be after awtLock
        {
            // use WWindowPeer instead of WDialogPeer because of FileDialogs and PrintDialogs
            WWindowPeer blockerPeer = (WWindowPeer)dialog.getPeer();
            if (blocked)
            {
                modalBlocker = blockerPeer;
                // handle native dialogs separately, as they may have not
                // got HWND yet; modalEnable/modalDisable is called from
                // their setHWnd() methods
                if (blockerPeer instanceof WFileDialogPeer) {
                    ((WFileDialogPeer)blockerPeer).blockWindow(this);
                } else if (blockerPeer instanceof WPrintDialogPeer) {
                    ((WPrintDialogPeer)blockerPeer).blockWindow(this);
                } else {
                    modalDisable(dialog, blockerPeer.getHWnd());
                }
            } else {
                modalBlocker = null;
                if (blockerPeer instanceof WFileDialogPeer) {
                    ((WFileDialogPeer)blockerPeer).unblockWindow(this);
                } else if (blockerPeer instanceof WPrintDialogPeer) {
                    ((WPrintDialogPeer)blockerPeer).unblockWindow(this);
                } else {
                    modalEnable(dialog);
                }
            }
        }
    }

    native void modalDisable(Dialog blocker, long blockerHWnd);
    native void modalEnable(Dialog blocker);

    /*
     * Returns all the ever active windows from the current AppContext.
     * The list is sorted by the time of activation, so the latest
     * active window is always at the end.
     */
    public static long[] getActiveWindowHandles() {
        AppContext appContext = AppContext.getAppContext();
        synchronized (appContext) {
            List<WWindowPeer> l = (List<WWindowPeer>)appContext.get(ACTIVE_WINDOWS_KEY);
            if (l == null) {
                return null;
            }
            long[] result = new long[l.size()];
            for (int j = 0; j < l.size(); j++) {
                result[j] = l.get(j).getHWnd();
            }
            return result;
        }
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

    public void updateGC() {
        int scrn = getScreenImOn();

        // get current GD
        Win32GraphicsDevice oldDev = (Win32GraphicsDevice)winGraphicsConfig
                                     .getDevice();

        Win32GraphicsDevice newDev;
        GraphicsDevice devs[] = GraphicsEnvironment
            .getLocalGraphicsEnvironment()
            .getScreenDevices();
        // Occasionally during device addition/removal getScreenImOn can return
        // a non-existing screen number. Use the default device in this case.
        if (scrn >= devs.length) {
            newDev = (Win32GraphicsDevice)GraphicsEnvironment
                .getLocalGraphicsEnvironment().getDefaultScreenDevice();
        } else {
            newDev = (Win32GraphicsDevice)devs[scrn];
        }

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
    }

    /*
     * From the DisplayChangedListener interface
     *
     * This method handles a display change - either when the display settings
     * are changed, or when the window has been dragged onto a different
     * display.
     */
    public void displayChanged() {
        updateGC();
        super.displayChanged();
    }

    private native int getScreenImOn();

/*
 * ----END DISPLAY CHANGE SUPPORT----
 */

     public void grab() {
         nativeGrab();
     }

     public void ungrab() {
         nativeUngrab();
     }
     private native void nativeGrab();
     private native void nativeUngrab();

     private final boolean hasWarningWindow() {
         return ((Window)target).getWarningString() != null;
     }

    boolean isTargetUndecorated() {
        return true;
    }

    // These are the peer bounds. They get updated at:
    //    1. the WWindowPeer.setBounds() method.
    //    2. the native code (on WM_SIZE/WM_MOVE)
    private volatile int sysX = 0;
    private volatile int sysY = 0;
    private volatile int sysW = 0;
    private volatile int sysH = 0;

    Rectangle constrainBounds(int x, int y, int width, int height) {
        GraphicsConfiguration gc = winGraphicsConfig;

        // We don't restrict the setBounds() operation if the code is trusted.
        if (!hasWarningWindow() || gc == null) {
            return new Rectangle(x, y, width, height);
        }

        int newX = x;
        int newY = y;
        int newW = width;
        int newH = height;

        Window target = (Window)this.target;

        Rectangle sB = gc.getBounds();
        Insets sIn = target.getToolkit().getScreenInsets(gc);

        int screenW = sB.width - sIn.left - sIn.right;
        int screenH = sB.height - sIn.top - sIn.bottom;

        // If it's undecorated or is not currently visible
        if (!visible || isTargetUndecorated()) {
            // Now check each point is within the visible part of the screen
            int screenX = sB.x + sIn.left;
            int screenY = sB.y + sIn.top;

            // First make sure the size is within the visible part of the screen
            if (newW > screenW) {
                newW = screenW;
            }
            if (newH > screenH) {
                newH = screenH;
            }

            // Tweak the location if needed
            if (newX < screenX) {
                newX = screenX;
            } else if (newX + newW > screenX + screenW) {
                newX = screenX + screenW - newW;
            } 
            if (newY < screenY) {
                newY = screenY;
            } else if (newY + newH > screenY + screenH) {
                newY = screenY + screenH - newH;
            }
        } else {
            int maxW = Math.max(screenW, sysW);
            int maxH = Math.max(screenH, sysH);

            // Make sure the size is within the visible part of the screen
            // OR less that the current size of the window.
            if (newW > maxW) {
                newW = maxW;
            }
            if (newH > maxH) {
                newH = maxH;
            }
        }

        return new Rectangle(newX, newY, newW, newH);
    }

     @Override
     public void setBounds(int x, int y, int width, int height, int op) {
         Rectangle newBounds = constrainBounds(x, y, width, height);

         sysX = newBounds.x;
         sysY = newBounds.y;
         sysW = newBounds.width;
         sysH = newBounds.height;

         super.setBounds(newBounds.x, newBounds.y, newBounds.width, newBounds.height, op);
     }


     @Override
     public void print(Graphics g) {
         // We assume we print the whole frame, 
         // so we expect no clip was set previously
         Shape shape = AWTAccessor.getWindowAccessor().getShape((Window)target);
         if (shape != null) {
             g.setClip(shape);
         }
         super.print(g);
     }

     private void replaceSurfaceDataRecursively(Component c) {
         if (c instanceof Container) {
             for (Component child : ((Container)c).getComponents()) {
                 replaceSurfaceDataRecursively(child);
             }
         }
         ComponentPeer cp = c.getPeer();
         if (cp instanceof WComponentPeer) {
             ((WComponentPeer)cp).replaceSurfaceDataLater();
         }
     }

     public final Graphics getTranslucentGraphics() {
         synchronized (getStateLock()) {
             return isOpaque ? null : painter.getBackBuffer(false).getGraphics();
         }
     }

     @Override
     public Graphics getGraphics() {
         synchronized (getStateLock()) {
             if (!isOpaque) {
                 return getTranslucentGraphics();
             }
         }
         return super.getGraphics();
     }


     private native void setOpacity(int iOpacity);
     private float opacity = 1.0f;

     public void setOpacity(float opacity) {
         if (!((SunToolkit)((Window)target).getToolkit()).
                 isWindowOpacityControlSupported()) 
         {
             return;
         }

         if (opacity < 0.0f || opacity > 1.0f) {
             throw new IllegalArgumentException(
                     "The value of opacity should be in the range [0.0f .. 1.0f].");
         }

         if (((this.opacity == 1.0f && opacity <  1.0f) ||
              (this.opacity <  1.0f && opacity == 1.0f)) &&
             !Win32GraphicsEnvironment.isVistaOS())
         {
             // non-Vista OS: only replace the surface data if opacity status
             // changed (see WComponentPeer.isAccelCapable() for more)
             replaceSurfaceDataRecursively((Component)getTarget());
         }

         this.opacity = opacity;

         final int maxOpacity = 0xff;
         int iOpacity = (int)(opacity * maxOpacity);
         if (iOpacity < 0) {
             iOpacity = 0;
         }
         if (iOpacity > maxOpacity) {
             iOpacity = maxOpacity;
         }

         setOpacity(iOpacity);
         updateWindow(true);
     }

     private native void setOpaqueImpl(boolean isOpaque);
     private boolean isOpaque = true;

     public void setOpaque(boolean isOpaque) {
         synchronized (getStateLock()) {
             if (this.isOpaque == isOpaque) {
                 return;
             }
         }

         final Window target = (Window)getTarget();

         if (!isOpaque) {
             final SunToolkit sunToolkit = (SunToolkit)target.getToolkit();
             if (!sunToolkit.isWindowTranslucencySupported() ||
                     !sunToolkit.isTranslucencyCapable(
                         target.getGraphicsConfiguration()))
             {
                 return;
             }
         }

         final boolean isVistaOS = Win32GraphicsEnvironment.isVistaOS();
         if (!isVistaOS){
             // non-Vista OS: only replace the surface data if the opacity
             // status changed (see WComponentPeer.isAccelCapable() for more)
             replaceSurfaceDataRecursively(target);
         }

         synchronized (getStateLock()) {
             this.isOpaque = isOpaque;

             setOpaqueImpl(isOpaque);

             if (isOpaque) {
                 TranslucentWindowPainter currentPainter = painter;
                 if (currentPainter != null) {
                     currentPainter.flush();
                     painter = null;
                 }
             } else {
                 painter = TranslucentWindowPainter.createInstance(this);
             }
         }

         if (isVistaOS) {
             // On Vista: setting the window non-opaque makes the window look
             // rectangular, though still catching the mouse clicks within
             // its shape only. To restore the correct visual appearance
             // of the window (i.e. w/ the correct shape) we have to reset
             // the shape.
             Shape shape = AWTAccessor.getWindowAccessor().getShape(target);
             if (shape != null) {
                 AWTAccessor.getWindowAccessor().setShape(target, shape);
             }
         }

         if (target.isVisible()) {
             updateWindow(true);
         }
     }

     native void updateWindowImpl(int[] data, int width, int height);

     public void updateWindow() {
         updateWindow(false);
     }

     private TranslucentWindowPainter painter;
     public void updateWindow(boolean repaint) {
         Window w = (Window)target;
         synchronized (getStateLock()) {
             if (isOpaque || !w.isVisible() ||
                     (w.getWidth() <= 0) || (w.getHeight() <= 0))
             {
                 return;
             }

             TranslucentWindowPainter currentPainter = painter;
             if (currentPainter!= null) {
                 currentPainter.updateWindow(repaint);
             } else if (log.isLoggable(Level.FINER)) {
                 log.log(Level.FINER,
                         "Translucent window painter is null in updateWindow");
             }
         }
     }

     public native void repositionSecurityWarning();

     @Override
     public void handleEvent(AWTEvent e) {
         if (!isOpaque && e.getID() == PaintEvent.UPDATE) {
             updateWindow(true);
         } else {
             super.handleEvent(e);
         }
     }

    /*
     * The method maps the list of the active windows to the window's AppContext,
     * then the method registers ActiveWindowListener, GuiDisposedListener listeners;
     * it executes the initilialization only once per AppContext.
     */
    private static void initActiveWindowsTracking(Window w) {
        AppContext appContext = AppContext.getAppContext();
        synchronized (appContext) {
            List<WWindowPeer> l = (List<WWindowPeer>)appContext.get(ACTIVE_WINDOWS_KEY);
            if (l == null) {
                l = new LinkedList<WWindowPeer>();
                appContext.put(ACTIVE_WINDOWS_KEY, l);
                appContext.addPropertyChangeListener(AppContext.GUI_DISPOSED, guiDisposedListener);
                
                KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
                kfm.addPropertyChangeListener("activeWindow", activeWindowListener);
            }
        }
    }

    /*
     * The GuiDisposedListener class listens for the AppContext.GUI_DISPOSED property,
     * it removes the list of the active windows from the disposed AppContext and
     * unregisters ActiveWindowListener listener.
     */
    private static class GuiDisposedListener implements PropertyChangeListener {
        public void propertyChange(PropertyChangeEvent e) {
            boolean isDisposed = (Boolean)e.getNewValue();
            if (isDisposed != true) {
                if (log.isLoggable(Level.FINE)) {
                    log.log(Level.FINE, " Assertion (newValue != true) failed for AppContext.GUI_DISPOSED ");
                }
            }
            AppContext appContext = AppContext.getAppContext();
            synchronized (appContext) {
                appContext.remove(ACTIVE_WINDOWS_KEY);
                appContext.removePropertyChangeListener(AppContext.GUI_DISPOSED, this);
                
                KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();            
                kfm.removePropertyChangeListener("activeWindow", activeWindowListener);
            }
        }
    }

    /*
     * Static inner class, listens for 'activeWindow' KFM property changes and
     * updates the list of active windows per AppContext, so the latest active
     * window is always at the end of the list. The list is stored in AppContext.
     */
    private static class ActiveWindowListener implements PropertyChangeListener {
        public void propertyChange(PropertyChangeEvent e) {
            Window w = (Window)e.getNewValue();
            if (w == null) {
                return;
            }
	    AppContext appContext = SunToolkit.targetToAppContext(w);
	    synchronized (appContext) {
                WWindowPeer wp = (WWindowPeer)w.getPeer();
                // Ignore fake notifications, jdk7 has 6566905 but it's too risky for 6u12
                boolean isValidNotification = w.isFocusableWindow() && w.isVisible() && w.isDisplayable();
                // add/move wp to the end of the list
		List<WWindowPeer> l = (List<WWindowPeer>)appContext.get(ACTIVE_WINDOWS_KEY);
                if (l != null && isValidNotification) {
                    l.remove(wp);
                    l.add(wp);
                }
            }
        }
    }
}
