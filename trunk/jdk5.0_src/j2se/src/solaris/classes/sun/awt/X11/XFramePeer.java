/*
 * @(#)XFramePeer.java	1.54 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.util.Vector;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.im.*;
import java.awt.image.ColorModel;
import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferUShort;
import java.awt.image.ImageObserver;
import java.awt.image.WritableRaster;
import sun.awt.image.ImageRepresentation;
import sun.java2d.SunGraphics2D;
import sun.awt.*;
import sun.awt.image.ToolkitImage;
import java.util.logging.*;
import java.lang.reflect.Field;

class XFramePeer extends XDecoratedPeer implements FramePeer, XConstants {
    private static Logger log = Logger.getLogger("sun.awt.X11.XFramePeer");
    private static Logger stateLog = Logger.getLogger("sun.awt.X11.states");
    private static Logger insLog = Logger.getLogger("sun.awt.X11.insets.XFramePeer");

    XIconWindow iconWindow;
    XMenuBarPeer menubarPeer;
    MenuBar menubar;
    int state;
    private Boolean undecorated;

    XFramePeer(Frame target) {
        super(target);
    }

    XFramePeer(XCreateWindowParams params) {
        super(params);
    }

    void preInit(XCreateWindowParams params) {
        super.preInit(params);
        Frame target = (Frame)(this.target);
        // set the window attributes for this Frame
        winAttr.initialState = target.getState();
        state = 0;
        winAttr.icon = target.getIconImage();
        undecorated = Boolean.valueOf(target.isUndecorated());
        winAttr.nativeDecor = !target.isUndecorated();
        if (winAttr.nativeDecor) {
            winAttr.decorations = winAttr.AWT_DECOR_ALL;
        } else {
            winAttr.decorations = winAttr.AWT_DECOR_NONE;
        }
        winAttr.isResizable = true; // target.isResizable();
        winAttr.title = target.getTitle();
        winAttr.initialResizability = target.isResizable();
        if (log.isLoggable(Level.FINE)) {
            log.log(Level.FINE, "Frame''s initial attributes: decor {0}, resizable {1}, undecorated {2}, initial state {3}",
                     new Object[] {new Integer(winAttr.decorations), Boolean.valueOf(winAttr.initialResizability),
                                   Boolean.valueOf(!winAttr.nativeDecor), new Integer(winAttr.initialState)});
        }
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        if (winAttr.icon != null) {
            setIconImage(winAttr.icon);
        }
        setupState(true);
        setIconName();
    }

    protected Insets guessInsets() {
        if (isTargetUndecorated()) {
            return new Insets(0, 0, 0, 0);
        } else {
            return super.guessInsets();
        }
    }

    private boolean isTargetUndecorated() {
        if (undecorated != null) {
            return undecorated.booleanValue();
        } else {
            return ((Frame)target).isUndecorated();
        }
    }

    String getIconName() {
        return getWMName();
    }
    
    void setIconName() {
        XAtom atom = XAtom.get(XAtom.XA_WM_ICON_NAME);
        String name = getIconName();
        if (name == null || name.trim().equals("")) {
            name = "Java";
        }
        atom.setProperty(getWindow(), name);
    }
    
    protected String getWMName() {
        if (winAttr.title == null || winAttr.title.trim().equals("")) {
            return " ";
        } else {
            return winAttr.title;
        }
    }

    void setupState(boolean onInit) {
        if (onInit) {
            state = winAttr.initialState;
        }
        if ((state & Frame.ICONIFIED) != 0) {
            setInitialState(IconicState);
        } else {
            setInitialState(NormalState);
        }
        setExtendedState(state);
    }

    public void setMenuBar(MenuBar mb) {
        // state_lock should always be the second after awt_lock
        XToolkit.awtLock();
        try {
            synchronized(getStateLock()) {
                if (mb == menubar) return;
                if (mb == null) {
                    if (menubar != null) {
                        menubarPeer.xSetVisible(false);
                        menubar = null;
                        menubarPeer.dispose();
                        menubarPeer = null;
                    }
                    return;
                }
                menubar = mb;
                menubarPeer = (XMenuBarPeer) mb.getPeer();
                if (menubarPeer != null) {
                    menubarPeer.init((Frame)target);
                }
                Rectangle r = target.getBounds();
                reshape(r.x, r.y, r.width, r.height);
                updateChildrenSizes();
            }
        } finally {
            XToolkit.awtUnlock();
        }
        if (target.isVisible()) {
            target.validate();
        }
    }

    XMenuBarPeer getMenubarPeer() {
        return menubarPeer;
    }

    int getMenuBarHeight() {
        if (menubarPeer != null) {
            return menubarPeer.getHeight();
        } else {
            return 0;
        }
    }
    void updateChildrenSizes() {
        super.updateChildrenSizes();
        // XWindow.reshape calls XBaseWindow.xSetBounds, which acquires
        // the getAWTLock(), so we have to acquire getAWTLock() here
        // before getStateLock() to avoid a deadlock with the Toolkit thread
        // when this method is called on the EDT.
        synchronized (getAWTLock()) {  
            synchronized(getStateLock()) {
                int width = dimensions.getClientSize().width;
                if (menubarPeer != null) {            
                    menubarPeer.reshape(0, 0, width, getMenuBarHeight());
                }
            }
        }
    }

    public void setIconImage(Image im) {
        if (iconWindow == null) {
            iconWindow = new XIconWindow(this);
        }
        int width;
        int height;
        if (im != null) {  // 4633887  Avoid Null pointer exception.
            if (im instanceof ToolkitImage) {
                ImageRepresentation ir = ((ToolkitImage)im).getImageRep();
                ir.reconstruct(ImageObserver.ALLBITS);
                width = ir.getWidth();
                height = ir.getHeight();
            }
            else {
                width = im.getWidth(null);
                height = im.getHeight(null);
            }
            Dimension iconSize = iconWindow.getIconSize(width, height);
            log.log(Level.FINER, "Icon size is {0}", new Object[]{iconSize});
            if (iconSize != null) {
                int iconWidth = iconSize.width;
                int iconHeight = iconSize.height;

                //Icons are displayed using the default visual, so create image
                //using default GraphicsConfiguration
                GraphicsConfiguration defaultGC = getGraphicsConfiguration().getDevice().
                    getDefaultConfiguration();
                ColorModel model = defaultGC.getColorModel();
                WritableRaster raster = 
                    model.createCompatibleWritableRaster(iconWidth, iconHeight);
                Image image = new BufferedImage(model, raster, 
                                                model.isAlphaPremultiplied(),
                                                null);
            
                // ARGB BufferedImage to hunt for transparent pixels
                BufferedImage bimage = 
                    new BufferedImage(iconWidth, iconHeight,
                                      BufferedImage.TYPE_INT_ARGB);
                ColorModel alphaCheck = bimage.getColorModel();
                Graphics g = image.getGraphics(); 
                Graphics big = bimage.getGraphics();
                try {
                    g.drawImage(im, 0, 0, iconWidth, iconHeight, null);
                    big.drawImage(im, 0, 0, iconWidth, iconHeight, null);
                } finally {
                    g.dispose();
                    big.dispose();
                }

                DataBuffer db = ((BufferedImage)image).getRaster().getDataBuffer();
                DataBuffer bidb = bimage.getRaster().getDataBuffer();
                byte[] bytedata = null;
                int[] intdata = null;
                int bidbLen = bidb.getSize();
                int imgDataIdx;
                //Get native RGB value for window background color
                //Should work for byte as well as int
                int bgRGB = getNativeColor(SystemColor.window, defaultGC);

                /* My first attempt at a solution to bug 4175560 was to use
                 * the iconMask and iconPixmap attributes of Windows.
                 * This worked fine on CDE/dtwm, however olwm displayed only
                 * single color icons (white on background).  Instead, the
                 * fix gets the default background window color and replaces
                 * transparent pixels in the icon image with this color.  This
                 * solutions works well with dtwm as well as olwm.
                 */

                for (imgDataIdx = 0; imgDataIdx < bidbLen; imgDataIdx++) {
                    if (alphaCheck.getAlpha(bidb.getElem(imgDataIdx)) == 0 ) {
                        //Assuming single data bank
                        db.setElem(imgDataIdx, bgRGB);
                    }
                }
                iconWindow.setIconImage(iconWidth, iconHeight, db);
            }
        }
    }

    public void setMaximizedBounds(Rectangle b) {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting maximized bounds to " + b);
        if (b == null) return;
        XToolkit.awtLock();
        maxBounds = new Rectangle(b);
        try {
            XSizeHints hints = getHints();
            hints.set_flags(hints.get_flags() | (int)XlibWrapper.PMaxSize);
            if (b.width != Integer.MAX_VALUE) {
                hints.set_max_width(b.width);
            } else {
                hints.set_max_width((int)XlibWrapper.DisplayWidth(XToolkit.getDisplay(), XlibWrapper.DefaultScreen(XToolkit.getDisplay())));
            }
            if (b.height != Integer.MAX_VALUE) {
                hints.set_max_height(b.height);
            } else {
                hints.set_max_height((int)XlibWrapper.DisplayHeight(XToolkit.getDisplay(), XlibWrapper.DefaultScreen(XToolkit.getDisplay())));
            }
            if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting hints, flags " + XlibWrapper.hintsToString(hints.get_flags()));
            XlibWrapper.XSetWMNormalHints(XToolkit.getDisplay(), window, hints.pData);
        } finally {
            XToolkit.awtUnlock();            
        }
    }

    public int getState() { return state; }

    public void setState(int newState) {
        if (!isShowing()) {
            stateLog.finer("Frame is not showing");
            state = newState;
            return;
        }
        changeState(newState);
    }
    
    void changeState(int newState) {
        int changed = state ^ newState;
        int changeIconic = changed & Frame.ICONIFIED;
        boolean iconic = (newState & Frame.ICONIFIED) != 0;
        stateLog.log(Level.FINER, "Changing state, old state {0}, new state {1}(iconic {2})", 
                     new Object[] {new Integer(state), new Integer(newState), new Boolean(iconic)});
        if (changeIconic != 0 && iconic) {
            if (stateLog.isLoggable(Level.FINER)) stateLog.finer("Iconifying shell " + getShell() + ", this " + this + ", screen " + getScreenNumber());
            try {
                XToolkit.awtLock();
                int res = XlibWrapper.XIconifyWindow(XToolkit.getDisplay(), getShell(), getScreenNumber());
                if (stateLog.isLoggable(Level.FINER)) stateLog.finer("XIconifyWindow returned " + res);
            }
            finally {
                XToolkit.awtUnlock();
            }
        }
        if ((changed & ~Frame.ICONIFIED) != 0) {
            setExtendedState(newState);
        }
        if (changeIconic != 0 && !iconic) {
            if (stateLog.isLoggable(Level.FINER)) stateLog.finer("DeIconifying " + this);
            xSetVisible(true);
        }
    }

    void setExtendedState(int newState) {
        XWM.getWM().setExtendedState(this, newState);
    }

    public void handlePropertyNotify(long event_ptr) {
        super.handlePropertyNotify(event_ptr);
        XPropertyEvent ev = new XPropertyEvent(event_ptr);

        log.log(Level.FINER, "Property change {0}", new Object[] {ev});
        /* 
         * Let's see if this is a window state protocol message, and
         * if it is - decode a new state in terms of java constants.
         */
        Integer newState = XWM.getWM().isStateChange(this, ev);
        if (newState == null) {
            return;
        }
        
        int changed = state ^ newState.intValue();
        if (changed == 0) {
            stateLog.finer("State is the same: " + state);
            return;
        }

        int old_state = state;
        state = newState.intValue();

        if ((changed & Frame.ICONIFIED) != 0) {
            if ((state & Frame.ICONIFIED) != 0) {
                stateLog.finer("Iconified");
                handleIconify();
            } else {
                stateLog.finer("DeIconified");
                handleDeiconify();
            }
        }        
        handleStateChange(old_state, state);
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleStateChange(int oldState, int newState) {
        postEvent(new WindowEvent((Window)target,
                                  WindowEvent.WINDOW_STATE_CHANGED,
                                  oldState, newState));
    }

    public void setVisible(boolean vis) {
        if (vis) {
            setupState(false);
        } else {
            if ((state & Frame.MAXIMIZED_BOTH) != 0) {
                XWM.getWM().setExtendedState(this, state & ~Frame.MAXIMIZED_BOTH);
            }
        }
        super.setVisible(vis);
        if (!vis && (state & Frame.ICONIFIED) != 0) { 
            // ICCCM, 4.1.4. Changing Window State:
            // "Iconic -> Withdrawn - The client should unmap the window and follow it 
            // with a synthetic UnmapNotify event as described later in this section."
            try {
                XToolkit.awtLock();
                XUnmapEvent unmap = new XUnmapEvent();
                unmap.set_window(getShell());
                unmap.set_event(XToolkit.getDefaultRootWindow());
                unmap.set_type((int)XlibWrapper.UnmapNotify);
                unmap.set_from_configure(false);
                XlibWrapper.XSendEvent(XToolkit.getDisplay(), XToolkit.getDefaultRootWindow(),
                        false, XlibWrapper.SubstructureNotifyMask | XlibWrapper.SubstructureRedirectMask,
                        unmap.pData);
                unmap.dispose();
            }
            finally {
                XToolkit.awtUnlock();
            }
        }
        if (vis && maxBounds != null) {
            setMaximizedBounds(maxBounds);
        }
    }

    void setInitialState(int wm_state) {
        try {
            XToolkit.awtLock();
            XWMHints hints = getWMHints();
            hints.set_flags((int)XlibWrapper.StateHint | hints.get_flags());
            hints.set_initial_state(wm_state);
            if (stateLog.isLoggable(Level.FINE)) stateLog.fine("Setting initial WM state on " + this + " to " + wm_state);
            XlibWrapper.XSetWMHints(XToolkit.getDisplay(), getWindow(), hints.pData);
        }
        finally {
            XToolkit.awtUnlock();        
        }
    }

    public void dispose() {
        if (menubarPeer != null) {
            menubarPeer.dispose();            
        }
        super.dispose();
    }
    
    boolean isMaximized() {
        return (state & (Frame.MAXIMIZED_VERT  | Frame.MAXIMIZED_HORIZ)) != 0;
    }

 


    static final int CROSSHAIR_INSET = 5;

    static final int BUTTON_Y = CROSSHAIR_INSET + 1;
    static final int BUTTON_W = 17;
    static final int BUTTON_H = 17;
  
    static final int SYS_MENU_X = CROSSHAIR_INSET + 1;
    static final int SYS_MENU_CONTAINED_X = SYS_MENU_X + 5;
    static final int SYS_MENU_CONTAINED_Y = BUTTON_Y + 7;
    static final int SYS_MENU_CONTAINED_W = 8;
    static final int SYS_MENU_CONTAINED_H = 3;
  
    static final int MAXIMIZE_X_DIFF = CROSSHAIR_INSET + BUTTON_W;
    static final int MAXIMIZE_CONTAINED_X_DIFF = MAXIMIZE_X_DIFF - 5;
    static final int MAXIMIZE_CONTAINED_Y = BUTTON_Y + 5;
    static final int MAXIMIZE_CONTAINED_W = 8;
    static final int MAXIMIZE_CONTAINED_H = 8;
  
    static final int MINIMIZE_X_DIFF = MAXIMIZE_X_DIFF + BUTTON_W;
    static final int MINIMIZE_CONTAINED_X_DIFF = MINIMIZE_X_DIFF - 7;
    static final int MINIMIZE_CONTAINED_Y = BUTTON_Y + 7;
    static final int MINIMIZE_CONTAINED_W = 3;
    static final int MINIMIZE_CONTAINED_H = 3;
  
    static final int TITLE_X = SYS_MENU_X + BUTTON_W;
    static final int TITLE_W_DIFF = BUTTON_W * 3 + CROSSHAIR_INSET * 2 - 1;
    static final int TITLE_MID_Y = BUTTON_Y + (BUTTON_H / 2);

    static final int MENUBAR_X = CROSSHAIR_INSET + 1;
    static final int MENUBAR_Y = BUTTON_Y + BUTTON_H;

    static final int HORIZ_RESIZE_INSET = CROSSHAIR_INSET + BUTTON_H;
    static final int VERT_RESIZE_INSET = CROSSHAIR_INSET + BUTTON_W;


    /*
     * Print the native component by rendering the Motif look ourselves.
     * We also explicitly print the MenuBar since a MenuBar isn't a subclass
     * of Component (and thus it has no "print" method which gets called by
     * default).
     */
    public void print(Graphics g) {
        super.print(g);

        Frame f = (Frame)target;
        Insets finsets = f.getInsets();
        Dimension fsize = f.getSize();

        Color bg = f.getBackground();
        Color fg = f.getForeground();
        Color highlight = bg.brighter();
        Color shadow = bg.darker();

        // Well, we could query for the currently running window manager
        // and base the look on that, or we could just always do dtwm.
        // aim, tball, and levenson all agree we'll just do dtwm.

        if (hasDecorations(XWindowAttributesData.AWT_DECOR_BORDER)) {

            // top outer -- because we'll most likely be drawing on white paper,
            // for aesthetic reasons, don't make any part of the outer border
            // pure white
            if (highlight.equals(Color.white)) {
                g.setColor(new Color(230, 230, 230));
            }
            else {
                g.setColor(highlight);
            }
            g.drawLine(0, 0, fsize.width, 0);
            g.drawLine(0, 1, fsize.width - 1, 1);
            
            // left outer
            // if (highlight.equals(Color.white)) {
            //     g.setColor(new Color(230, 230, 230));
            // }
            // else {
            //     g.setColor(highlight);
            // }
            g.drawLine(0, 0, 0, fsize.height);
            g.drawLine(1, 0, 1, fsize.height - 1);
            
            // bottom cross-hair
            g.setColor(highlight);
            g.drawLine(CROSSHAIR_INSET + 1, fsize.height - CROSSHAIR_INSET,
                       fsize.width - CROSSHAIR_INSET, 
                       fsize.height - CROSSHAIR_INSET);
            
            // right cross-hair
            // g.setColor(highlight);
            g.drawLine(fsize.width - CROSSHAIR_INSET, CROSSHAIR_INSET + 1,
                       fsize.width - CROSSHAIR_INSET,
                       fsize.height - CROSSHAIR_INSET);
            
            // bottom outer
            g.setColor(shadow);
            g.drawLine(1, fsize.height, fsize.width, fsize.height);
            g.drawLine(2, fsize.height - 1, fsize.width, fsize.height - 1);

            // right outer
            // g.setColor(shadow);
            g.drawLine(fsize.width, 1, fsize.width, fsize.height);
            g.drawLine(fsize.width - 1, 2, fsize.width - 1, fsize.height);

            // top cross-hair
            // g.setColor(shadow);
            g.drawLine(CROSSHAIR_INSET, CROSSHAIR_INSET, 
                       fsize.width - CROSSHAIR_INSET, CROSSHAIR_INSET);

            // left cross-hair
            // g.setColor(shadow);
            g.drawLine(CROSSHAIR_INSET, CROSSHAIR_INSET, CROSSHAIR_INSET,
                       fsize.height - CROSSHAIR_INSET);
        }

        if (hasDecorations(XWindowAttributesData.AWT_DECOR_TITLE)) {

            if (hasDecorations(XWindowAttributesData.AWT_DECOR_MENU)) {

                // system menu
                g.setColor(bg);
                g.fill3DRect(SYS_MENU_X, BUTTON_Y, BUTTON_W, BUTTON_H, true);
                g.fill3DRect(SYS_MENU_CONTAINED_X, SYS_MENU_CONTAINED_Y,
                             SYS_MENU_CONTAINED_W, SYS_MENU_CONTAINED_H, true);
            }

            // title bar
            // g.setColor(bg);
            g.fill3DRect(TITLE_X, BUTTON_Y, fsize.width - TITLE_W_DIFF, BUTTON_H,
                         true);
        
            if (hasDecorations(XWindowAttributesData.AWT_DECOR_MINIMIZE)) {

                // minimize button
                // g.setColor(bg);
                g.fill3DRect(fsize.width - MINIMIZE_X_DIFF, BUTTON_Y, BUTTON_W,
                             BUTTON_H, true);
                g.fill3DRect(fsize.width - MINIMIZE_CONTAINED_X_DIFF,
                             MINIMIZE_CONTAINED_Y, MINIMIZE_CONTAINED_W,
                             MINIMIZE_CONTAINED_H, true);
            }

            if (hasDecorations(XWindowAttributesData.AWT_DECOR_MAXIMIZE)) {

                // maximize button
                // g.setColor(bg);
                g.fill3DRect(fsize.width - MAXIMIZE_X_DIFF, BUTTON_Y, BUTTON_W,
                             BUTTON_H, true);
                g.fill3DRect(fsize.width - MAXIMIZE_CONTAINED_X_DIFF,
                             MAXIMIZE_CONTAINED_Y, MAXIMIZE_CONTAINED_W,
                             MAXIMIZE_CONTAINED_H, true);
            }

            // title bar text
            g.setColor(fg);
            Font sysfont = new Font("SansSerif", Font.PLAIN, 10);
            g.setFont(sysfont);
            FontMetrics sysfm = g.getFontMetrics();
            String ftitle = f.getTitle();
            g.drawString(ftitle, 
                         ((TITLE_X + TITLE_X + fsize.width - TITLE_W_DIFF) / 2) - 
                         (sysfm.stringWidth(ftitle) / 2),
                         TITLE_MID_Y + sysfm.getMaxDescent());
        }
            
        if (f.isResizable() && 
            hasDecorations(XWindowAttributesData.AWT_DECOR_RESIZEH)) {

            // add resize cross hairs

            // upper-left horiz (shadow)
            g.setColor(shadow);
            g.drawLine(1, HORIZ_RESIZE_INSET, CROSSHAIR_INSET,
                       HORIZ_RESIZE_INSET);
            // upper-left vert (shadow)
            // g.setColor(shadow);
            g.drawLine(VERT_RESIZE_INSET, 1, VERT_RESIZE_INSET, CROSSHAIR_INSET);
            // upper-right horiz (shadow)
            // g.setColor(shadow);
            g.drawLine(fsize.width - CROSSHAIR_INSET + 1, HORIZ_RESIZE_INSET,
                       fsize.width, HORIZ_RESIZE_INSET);
            // upper-right vert (shadow)
            // g.setColor(shadow);
            g.drawLine(fsize.width - VERT_RESIZE_INSET - 1, 2,
                       fsize.width - VERT_RESIZE_INSET - 1, CROSSHAIR_INSET + 1);
            // lower-left horiz (shadow)
            // g.setColor(shadow);
            g.drawLine(1, fsize.height - HORIZ_RESIZE_INSET - 1,
                       CROSSHAIR_INSET, fsize.height - HORIZ_RESIZE_INSET - 1);
            // lower-left vert (shadow)
            // g.setColor(shadow);
            g.drawLine(VERT_RESIZE_INSET, fsize.height - CROSSHAIR_INSET + 1,
                       VERT_RESIZE_INSET, fsize.height);
            // lower-right horiz (shadow)
            // g.setColor(shadow);
            g.drawLine(fsize.width - CROSSHAIR_INSET + 1,
                       fsize.height - HORIZ_RESIZE_INSET - 1, fsize.width,
                       fsize.height - HORIZ_RESIZE_INSET - 1);
            // lower-right vert (shadow)
            // g.setColor(shadow);
            g.drawLine(fsize.width - VERT_RESIZE_INSET - 1,
                       fsize.height - CROSSHAIR_INSET + 1,
                       fsize.width - VERT_RESIZE_INSET - 1, fsize.height);

            // upper-left horiz (highlight)
            g.setColor(highlight);
            g.drawLine(2, HORIZ_RESIZE_INSET + 1, CROSSHAIR_INSET,
                       HORIZ_RESIZE_INSET + 1);
            // upper-left vert (highlight)
            // g.setColor(highlight);
            g.drawLine(VERT_RESIZE_INSET + 1, 2, VERT_RESIZE_INSET + 1,
                       CROSSHAIR_INSET);
            // upper-right horiz (highlight)
            // g.setColor(highlight);
            g.drawLine(fsize.width - CROSSHAIR_INSET + 1,
                       HORIZ_RESIZE_INSET + 1, fsize.width - 1,
                       HORIZ_RESIZE_INSET + 1);
            // upper-right vert (highlight)
            // g.setColor(highlight);
            g.drawLine(fsize.width - VERT_RESIZE_INSET, 2, 
                       fsize.width - VERT_RESIZE_INSET, CROSSHAIR_INSET);
            // lower-left horiz (highlight)
            // g.setColor(highlight);
            g.drawLine(2, fsize.height - HORIZ_RESIZE_INSET, CROSSHAIR_INSET,
                       fsize.height - HORIZ_RESIZE_INSET);
            // lower-left vert (highlight)
            // g.setColor(highlight);
            g.drawLine(VERT_RESIZE_INSET + 1,
                       fsize.height - CROSSHAIR_INSET + 1, 
                       VERT_RESIZE_INSET + 1, fsize.height - 1);
            // lower-right horiz (highlight)
            // g.setColor(highlight);
            g.drawLine(fsize.width - CROSSHAIR_INSET + 1,
                       fsize.height - HORIZ_RESIZE_INSET, fsize.width - 1,
                       fsize.height - HORIZ_RESIZE_INSET);
            // lower-right vert (highlight)
            // g.setColor(highlight);
            g.drawLine(fsize.width - VERT_RESIZE_INSET,
                       fsize.height - CROSSHAIR_INSET + 1,
                       fsize.width - VERT_RESIZE_INSET, fsize.height - 1);
        }

        XMenuBarPeer peer = menubarPeer;
        if (peer != null) {
            Insets insets = getInsets();
            Graphics ng = g.create();
            int menubarX = 0;
            int menubarY = 0;
            if (hasDecorations(XWindowAttributesData.AWT_DECOR_BORDER)) {
                menubarX += CROSSHAIR_INSET + 1;
                    menubarY += CROSSHAIR_INSET + 1;
            }
            if (hasDecorations(XWindowAttributesData.AWT_DECOR_TITLE)) {
                menubarY += BUTTON_H;
            }
            try {
                ng.translate(menubarX, menubarY);
                peer.print(ng);
            } finally {
                ng.dispose();
            }
        }
    }

    public void setBoundsPrivate(int x, int y, int width, int height) {
        setBounds(x, y, width, height, SET_BOUNDS);
    }
}
