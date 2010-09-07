/*
 * @(#)XBaseWindow.java	1.39 04/01/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import sun.awt.*;
import java.util.logging.*;

public class XBaseWindow implements XConstants, XUtilConstants {
    private static final Logger log = Logger.getLogger("sun.awt.X11.XBaseWindow");
    private static final Logger insLog = Logger.getLogger("sun.awt.X11.insets.XBaseWindow");
    private static final Logger eventLog = Logger.getLogger("sun.awt.X11.event.XBaseWindow");
    private static final Logger focusLog = Logger.getLogger("sun.awt.X11.focus.XBaseWindow");

    public static final String 
        PARENT_WINDOW = "parent window", // parent window, Long
        BOUNDS = "bounds", // bounds of the window, Rectangle
        OVERRIDE_REDIRECT = "overrideRedirect", // override_redirect setting, Boolean
        EVENT_MASK = "event mask", // event mask, Integer
        VALUE_MASK = "value mask", // value mask, Long
        BORDER_PIXEL = "border pixel", // border pixel value, Integer
        COLORMAP = "color map", // color map, Long
        DEPTH = "visual depth", // depth, Integer
        VISUAL_CLASS = "visual class", // visual class, Integer
        VISUAL = "visual", // visual, Long
        EMBEDDED = "embedded", // is embedded?, Boolean
        DELAYED = "delayed", // is creation delayed?, Boolean
        PARENT = "parent", // parent 
        BACKGROUND_PIXMAP = "pixmap", // background pixmap
        VISIBLE = "visible", // whether it is visible by default
        SAVE_UNDER = "save under", // save content under this window
        BACKING_STORE = "backing store", // enables double buffering
        BIT_GRAVITY = "bit gravity"; // copy old content on geometry change        
    private XCreateWindowParams delayedParams;

    long window;
    boolean visible;
    boolean mapped;
    boolean embedded;
    Rectangle maxBounds;
    private boolean disposed;

    private long screen;
    private XSizeHints hints;
    private XWMHints wmHints;


    final static int MIN_SIZE = 1;
    final static int DEF_LOCATION = 1;

    private static XAtom wm_client_leader;

    int x;
    int y;
    int width;
    int height;

    // To prevent errors from overriding obsolete methods
    protected final void init(long parentWindow, Rectangle bounds) {}
    protected final void preInit() {}
    protected final void postInit() {}

    // internal lock for synchronizing state changes and paint calls, initialized in preInit.
    // the order with other locks: AWTLock -> stateLock
    static class StateLock extends Object { }
    protected StateLock state_lock;

    /**
     * Called for delayed inits during construction
     */
    void instantPreInit(XCreateWindowParams params) {
        state_lock = new StateLock();
    }

    /**
     * Called before window creation, descendants should override to initialize the data,
     * initialize params.
     */
    void preInit(XCreateWindowParams params) {
        state_lock = new StateLock();
        embedded = Boolean.TRUE.equals(params.get(EMBEDDED));
        visible = Boolean.TRUE.equals(params.get(VISIBLE));
        screen = -1;
    }

    /**
     * Called after window creation, descendants should override to initialize Window
     * with class-specific values and perform post-initialization actions.
     */
    void postInit(XCreateWindowParams params) {
        if (log.isLoggable(Level.FINE)) log.fine("WM name is " + getWMName());
        setWMName(getWMName());

        // Set WM_CLIENT_LEADER property
        initClientLeader();
    }

    static final Object getAWTLock() {
        return XToolkit.getAWTLock();
    }
    
    /**
     * Creates window using parameters <code>params</code>
     * If params contain flag DELAYED doesn't do anything.
     * Note: Descendants can call this method to create the window
     * at the time different to instance construction.
     */
    protected final void init(XCreateWindowParams params) {
        if (!Boolean.TRUE.equals(params.get(DELAYED))) {
            preInit(params);
            create(params);
            postInit(params);
        } else {
            instantPreInit(params);
            delayedParams = params;
        }
    }

    /*
     * Creates an invisible InputOnly window without an associated Component.
     */
    XBaseWindow() {
        this(new XCreateWindowParams());
    }
    
    /**
     * Creates normal child window
     */
    XBaseWindow(long parentWindow, Rectangle bounds) {
        this(new XCreateWindowParams(new Object[] {
            BOUNDS, bounds,
            PARENT_WINDOW, new Long(parentWindow)}));
    }    
    
    /**
     * Creates top-level window
     */ 
    XBaseWindow(Rectangle bounds) {
        this(new XCreateWindowParams(new Object[] {
            BOUNDS, bounds
        }));
    }

    public XBaseWindow (XCreateWindowParams params) {
        init(params);
    }

    /* This create is used by the XEmbeddedFramePeer since it has to create the window
       as a child of the netscape window. This netscape window is passed in as wid */
    XBaseWindow(long parentWindow) {
        this(new XCreateWindowParams(new Object[] {
            PARENT_WINDOW, new Long(parentWindow),
            EMBEDDED, Boolean.TRUE
        }));        
    }

    /**
     * Verifies that all required parameters are set. If not, sets them to default values.
     * Verifies values of critical parameters, adjust their values when needed.
     * @throws IllegalArgumentException if params is null
     */ 
    protected void checkParams(XCreateWindowParams params) {
        if (params == null) {
            throw new IllegalArgumentException("Window creation parameters are null");
        }
        params.putIfNull(PARENT_WINDOW, new Long(XToolkit.getDefaultRootWindow()));
        params.putIfNull(BOUNDS, new Rectangle(DEF_LOCATION, DEF_LOCATION, MIN_SIZE, MIN_SIZE));
        params.putIfNull(DEPTH, new Integer((int)XlibWrapper.CopyFromParent));
        params.putIfNull(VISUAL, new Long(XlibWrapper.CopyFromParent));
        params.putIfNull(VISUAL_CLASS, new Integer((int)XlibWrapper.InputOnly));
        params.putIfNull(VALUE_MASK, new Long(XlibWrapper.CWEventMask));
        Rectangle bounds = (Rectangle)params.get(BOUNDS);
        bounds.width = Math.max(MIN_SIZE, bounds.width);
        bounds.height = Math.max(MIN_SIZE, bounds.height);

        Boolean overrideRedirect = (Boolean)params.get(OVERRIDE_REDIRECT);
        if (Boolean.TRUE.equals(overrideRedirect)) {
            params.put(PARENT_WINDOW, new Long(XToolkit.getDefaultRootWindow()));
        }
        
        Long eventMaskObj = (Long)params.get(EVENT_MASK);
        long eventMask = eventMaskObj != null ? eventMaskObj.longValue() : 0;
        // We use our own synthetic grab see XAwtState.getGrabWindow()
        // (see X vol. 1, 8.3.3.2)
        eventMask |= PropertyChangeMask | OwnerGrabButtonMask;
        params.put(EVENT_MASK, new Long(eventMask));
    }

    /**
     * Creates window with parameters specified by <code>params</code>
     * @see #init
     */
    private final void create(XCreateWindowParams params) {
        synchronized (getAWTLock()) {
            XSetWindowAttributes xattr = new XSetWindowAttributes();
            try {            
                checkParams(params);

                long value_mask = ((Long)params.get(VALUE_MASK)).longValue();

                Long eventMask = (Long)params.get(EVENT_MASK);
                xattr.set_event_mask(eventMask.longValue());
                value_mask |= XlibWrapper.CWEventMask;

                Long border_pixel = (Long)params.get(BORDER_PIXEL);
                if (border_pixel != null) {
                    xattr.set_border_pixel(border_pixel.longValue());
                    value_mask |= XlibWrapper.CWBorderPixel;
                }

                Long colormap = (Long)params.get(COLORMAP);
                if (colormap != null) {
                    xattr.set_colormap(colormap.longValue());
                    value_mask |= XlibWrapper.CWColormap;
                }
                Long background_pixmap = (Long)params.get(BACKGROUND_PIXMAP);
                if (background_pixmap != null) {
                    xattr.set_background_pixmap(background_pixmap.longValue());
                    value_mask |= XlibWrapper.CWBackPixmap;
                }

                Long parentWindow = (Long)params.get(PARENT_WINDOW);
                Rectangle bounds = (Rectangle)params.get(BOUNDS);
                Integer depth = (Integer)params.get(DEPTH);
                Integer visual_class = (Integer)params.get(VISUAL_CLASS);
                Long visual = (Long)params.get(VISUAL);
                Boolean overrideRedirect = (Boolean)params.get(OVERRIDE_REDIRECT);
                if (overrideRedirect != null) {
                    xattr.set_override_redirect(overrideRedirect.booleanValue());
                    value_mask |= XlibWrapper.CWOverrideRedirect;                
                }

                Boolean saveUnder = (Boolean)params.get(SAVE_UNDER);
                if (saveUnder != null) {
                    xattr.set_save_under(saveUnder.booleanValue());
                    value_mask |= XlibWrapper.CWSaveUnder;
                }

                Integer backingStore = (Integer)params.get(BACKING_STORE);
                if (backingStore != null) {
                    xattr.set_backing_store(backingStore.intValue());
                    value_mask |= XlibWrapper.CWBackingStore;
                }

                Integer bitGravity = (Integer)params.get(BIT_GRAVITY);
                if (bitGravity != null) {
                    xattr.set_bit_gravity(bitGravity.intValue());
                    value_mask |= XlibWrapper.CWBitGravity;
                }

                if (log.isLoggable(Level.FINE)) {
                    log.fine("Creating window for " + this + " with the following attributes: \n" + params);
                }
                window = XlibWrapper.XCreateWindow(XToolkit.getDisplay(),
                                   parentWindow.longValue(),
                                   bounds.x, bounds.y, // location
                                   bounds.width, bounds.height, // size
                                   0, // border
                                   depth.intValue(), // depth
                                   visual_class.intValue(), // class
                                   visual.longValue(), // visual
                                   value_mask,  // value mask
                                   xattr.pData); // attributes

                if (window == 0) {
                    throw new IllegalStateException("Couldn't create window because of wrong parameters. Run with NOISY_AWT to see details");
                }
                XToolkit.addToWinMap(window, this);            
            } finally {
                xattr.dispose();
            }
        }            
    }
    
    public XCreateWindowParams getDelayedParams() {
        return delayedParams;
    }

    protected String getWMName() {
        return XToolkit.getCorrectXIDString(getClass().getName());
    }

    protected void initClientLeader() {
        synchronized (getAWTLock()) {
            if (wm_client_leader == null) {
                wm_client_leader = XAtom.get("WM_CLIENT_LEADER");
            }
            wm_client_leader.setWindowProperty(this, getXAWTRootWindow());
        }
    }

    static XRootWindow getXAWTRootWindow() {
        return XRootWindow.getInstance();
    }

    void destroy() {
        synchronized (getAWTLock()) {
            if (hints != null) {
                XlibWrapper.XFree(hints.pData);
                hints = null;
            }
            XToolkit.removeFromWinMap(getWindow(), this);
            XlibWrapper.XDestroyWindow(XToolkit.getDisplay(), getWindow());
            window = -1;
            if( !isDisposed() ) {
                setDisposed( true );
            }
        }
    }

    void flush() {
        synchronized (getAWTLock()) {
            XlibWrapper.XFlush(XToolkit.getDisplay());
        }  
    }
    
    /**
     * Helper function to set W
     */
    public final void setWMHints(XWMHints hints) {
        synchronized(getAWTLock()) {
            XlibWrapper.XSetWMHints(XToolkit.getDisplay(), getWindow(), hints.pData);
        }
    }

    public XWMHints getWMHints() {
        if (wmHints == null) {
            wmHints = new XWMHints(XlibWrapper.XAllocWMHints());
//              XlibWrapper.XGetWMHints(XToolkit.getDisplay(),
//                                      getWindow(),
//                                      wmHints.pData);
        }
        return wmHints;
    }


    /* 
     * Call this method under AWTLock.
     * The lock should be acquired untill all operations with XSizeHints are completed.
     */
    public XSizeHints getHints() {
        if (hints == null) {
            long p_hints = XlibWrapper.XAllocSizeHints();
            hints = new XSizeHints(p_hints);
//              XlibWrapper.XGetWMNormalHints(XToolkit.getDisplay(), getWindow(), p_hints, XlibWrapper.larg1);
            // TODO: Shouldn't we listen for WM updates on this property?
        }
        return hints;
    }

    public void setSizeHints(long flags, int x, int y, int width, int height) {
        if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting hints, flags " + XlibWrapper.hintsToString(flags));
        synchronized (getAWTLock()) {
            XSizeHints hints = getHints();
            if ((flags & XlibWrapper.PPosition) != 0) {
                hints.set_x(x);
                hints.set_y(y);
            }
            if ((flags & XlibWrapper.PSize) != 0) {
                hints.set_width(width);
                hints.set_height(height);
            }
            if ((flags & XlibWrapper.PMinSize) != 0) {
                hints.set_min_width(width);
                hints.set_min_height(height);
            } else if ((hints.get_flags() & XlibWrapper.PMinSize) != 0) {
                flags |= XlibWrapper.PMinSize;
                hints.set_min_width(XlibWrapper.MINSIZE);
                hints.set_min_height(XlibWrapper.MINSIZE);
            }
            if ((flags & XlibWrapper.PMaxSize) != 0) {
                if (maxBounds != null) {
                    if (maxBounds.width != Integer.MAX_VALUE) {
                        hints.set_max_width(maxBounds.width);
                    } else {
                        hints.set_max_width(XToolkit.getDefaultScreenWidth());
                    }
                    if (maxBounds.height != Integer.MAX_VALUE) {
                        hints.set_max_height(maxBounds.height);
                    } else {
                        hints.set_max_height(XToolkit.getDefaultScreenHeight());
                    }
                } else {
                    hints.set_max_width(width);
                    hints.set_max_height(height);
                }
            } else if ((hints.get_flags() & XlibWrapper.PMaxSize) != 0) {
                flags |= XlibWrapper.PMaxSize;
                if (maxBounds != null) {
                    if (maxBounds.width != Integer.MAX_VALUE) {
                        hints.set_max_width(maxBounds.width);
                    } else {
                        hints.set_max_width(XToolkit.getDefaultScreenWidth());
                    }
                    if (maxBounds.height != Integer.MAX_VALUE) {
                        hints.set_max_height(maxBounds.height);
                    } else {
                        hints.set_max_height(XToolkit.getDefaultScreenHeight());
                    }
                } else {
                    // Leave intact
                }
            }
            flags |= XlibWrapper.PWinGravity;
            hints.set_flags(flags);
            hints.set_win_gravity((int)XlibWrapper.NorthWestGravity);
            if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting hints, resulted flags " + XlibWrapper.hintsToString(flags) + 
                                                             ", values " + hints);
            XlibWrapper.XSetWMNormalHints(XToolkit.getDisplay(), getWindow(), hints.pData);        
        }
    }

    /**
     * This lock object can be used to protect instance data from concurrent access
     * by two threads. If both state lock and AWT lock are taken, AWT Lock should be taken first.
     */
    Object getStateLock() {
        return state_lock;
    }

    public long getWindow() {
        return window;
    }
    public long getContentWindow() {
        return window;
    }

    public Rectangle getBounds() {
        return new Rectangle(x, y, width, height);
    }
    public Dimension getSize() {
        return new Dimension(width, height);
    }


    public void toFront() {
        synchronized (getAWTLock()) {
            XlibWrapper.XRaiseWindow(XToolkit.getDisplay(), getWindow());
        }
    }
    public void xRequestFocus(long time) {
        synchronized (getAWTLock()) {
            if (focusLog.isLoggable(Level.FINER)) focusLog.finer("XSetInputFocus on " + getWindow() + " with time " + time);
            XlibWrapper.XSetInputFocus2(XToolkit.getDisplay(), getWindow(), time);
        }
    }
    public void xRequestFocus() {
        synchronized (getAWTLock()) {
            if (focusLog.isLoggable(Level.FINER)) focusLog.finer("XSetInputFocus on " + getWindow());
             XlibWrapper.XSetInputFocus(XToolkit.getDisplay(), getWindow());
        }
    }

    public long xGetInputFocus() {
        synchronized (getAWTLock()) {
            return XlibWrapper.XGetInputFocus(XToolkit.getDisplay());
        }
    }

    public void xSetVisible(boolean visible) {
        if (log.isLoggable(Level.FINE)) log.fine("Setting visible on " + this + " to " + visible);
        synchronized (getAWTLock()) {
            this.visible = visible;
            if (visible) {
                XlibWrapper.XMapWindow(XToolkit.getDisplay(), getWindow());
            }
            else {
                XlibWrapper.XUnmapWindow(XToolkit.getDisplay(), getWindow());
            }        
            XlibWrapper.XFlush(XToolkit.getDisplay());
        }
    }

    boolean isMapped() {
        return mapped;
    }

    void setWMName(String name) {
        XAtom xa;
        synchronized (getAWTLock()) {
            xa = XAtom.get(XAtom.XA_WM_NAME);
            if (name == null) {
                name = " ";
            }
            xa.setProperty(getWindow(), name);
        }        
    }
    void setWMClass(String[] cl) {
        if (cl.length != 2) {
            throw new IllegalArgumentException("WM_CLASS_NAME consists of exactly two strings");
        }
        synchronized (getAWTLock()) {
            XAtom xa = XAtom.get(XAtom.XA_WM_CLASS);
            xa.setProperty8(getWindow(), cl[0] + '\0' + cl[1]);
        }                
    }

    boolean isVisible() {
        return visible;
    }

    static long getScreenOfWindow(long window) {
        synchronized (getAWTLock()) {
            return XlibWrapper.getScreenOfWindow(XToolkit.getDisplay(), window);
        }
    }
    long getScreenNumber() {
        synchronized (getAWTLock()) {
            return XlibWrapper.XScreenNumberOfScreen(getScreen());
        }
    }

    long getScreen() {
        if (screen == -1) { // Not initialized
            screen = getScreenOfWindow(window);
        }
        return screen;
    }

    public void xSetBounds(Rectangle bounds) {
        xSetBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    }

    public void xSetBounds(int x, int y, int width, int height) {
        if (getWindow() == 0) {
            insLog.warning("Attempt to resize uncreated window");
            throw new IllegalStateException("Attempt to resize uncreated window");
        }
        insLog.fine("Setting bounds on " + this + " to (" + x + ", " + y + "), " + width + "x" + height);
        if (width <= 0) {
            width = 1;
        }
        if (height <= 0) {
            height = 1;
        }
        synchronized (getAWTLock()) {
             XlibWrapper.XMoveResizeWindow(XToolkit.getDisplay(), getWindow(), x,y,width,height);
        }
    }

    /**
     * Translate coordinates
     */
    static Point toOtherWindow(long src, long dst, int x, int y) {
	Point rpt = new Point(0,0);

	synchronized (getAWTLock()) {
	    XlibWrapper.XTranslateCoordinates(XToolkit.getDisplay(),
		src, dst, x, y,
		XlibWrapper.larg1, XlibWrapper.larg2, XlibWrapper.larg3);

            rpt.x = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg1);
            rpt.y = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg2);
            return rpt;
        }
    }

    /*
     * Convert to global coordinates.
     */
    Rectangle toGlobal(Rectangle rec) {
	rec.setLocation(toGlobal(rec.getLocation()));
	return rec;
    }

    Point toGlobal(Point pt) {
        return toGlobal(pt.x, pt.y);
    }

    Point toGlobal(int x, int y) {
        long root;
        synchronized (getAWTLock()) {
            root = XlibWrapper.RootWindow(XToolkit.getDisplay(),
                    getScreenNumber());
        }
	return toOtherWindow(getContentWindow(), root, x, y);
    }

    /*
     * Convert to local coordinates.
     */
    Point toLocal(Point pt) {
	return toLocal(pt.x, pt.y);
    }

    Point toLocal(int x, int y) {
        long root;
        synchronized (getAWTLock()) {
            root = XlibWrapper.RootWindow(XToolkit.getDisplay(),
                    getScreenNumber());
        }
	return toOtherWindow(root, getContentWindow(), x, y);
    }

    /**
     * We should always grab both keyboard and pointer to control event flow 
     * on popups. This also simplifies synthetic grab implementation.
     * The active grab overrides activated automatic grab.
     */
    public boolean grabInput() {
        synchronized (getAWTLock()) {
            if (XAwtState.getGrabWindow() == this &&
                XAwtState.isManualGrab()) 
            {
                return true;
            }
            final int eventMask = (int) (ButtonPressMask | ButtonReleaseMask 
                | EnterWindowMask | LeaveWindowMask | PointerMotionMask 
                | ButtonMotionMask);
            final int ownerEvents = 1;
                
            int keyGrab = XlibWrapper.XGrabPointer(XToolkit.getDisplay(),  
                getContentWindow(), ownerEvents, eventMask, GrabModeAsync, 
                GrabModeAsync, None, XToolkit.arrowCursor, CurrentTime);
            // Check grab results to be consistent with X server grab
            if (keyGrab != GrabSuccess) {
                XAwtState.setGrabWindow(null);
                return false;
            }
            
            int ptrGrab = XlibWrapper.XGrabKeyboard(XToolkit.getDisplay(),  
                getContentWindow(), ownerEvents, GrabModeAsync, GrabModeAsync,
                CurrentTime);
            if (ptrGrab != GrabSuccess) {
                XlibWrapper.XUngrabKeyboard(XToolkit.getDisplay(), CurrentTime);
                XAwtState.setGrabWindow(null);
                return false;
            }
            XAwtState.setGrabWindow(this);
            return true;
        }
    }
    
    static void ungrabInput() {
        synchronized (getAWTLock()) {
           if (XAwtState.getGrabWindow() != null) {
                XlibWrapper.XUngrabPointer(XToolkit.getDisplay(), CurrentTime);
                XlibWrapper.XUngrabKeyboard(XToolkit.getDisplay(), CurrentTime);
                XAwtState.setGrabWindow(null);
           }
        }
    }    

    static void checkSecurity() {
        if (XToolkit.isSecurityWarningEnabled() && XToolkit.isToolkitThread()) {
            StackTraceElement stack[] = (new Throwable()).getStackTrace();
            log.warning(stack[1] + ": Security violation: calling user code on toolkit thread");
        }
    }

    // -------------- Event handling ----------------
    public void handleMapNotifyEvent(long ptr) {
        mapped = true;
    }
    public void handleUnmapNotifyEvent(long ptr) {
        mapped = false;
    }
    public void handleReparentNotifyEvent(long ptr) {
    }
    public void handlePropertyNotify(long ptr) {
        if (eventLog.isLoggable(Level.FINER)) {
            XPropertyEvent msg = new XPropertyEvent(ptr);
            eventLog.finer(msg.toString());
        }        
    }
    public void handleDestroyNotify(long ptr) {        
        XDestroyWindowEvent de = new XDestroyWindowEvent(ptr);
        if (de.get_window() == getWindow()) {
            XToolkit.removeFromWinMap(getWindow(), this);
        }
    }

    public void handleClientMessage(long ptr) {
        if (eventLog.isLoggable(Level.FINER)) {
            XClientMessageEvent msg = new XClientMessageEvent(ptr);
            eventLog.finer(msg.toString());
        }
    }

    public void handleVisibilityEvent(long ptr) {
    }
    public void handleKeyPress(long ptr) {
    }
    public void handleKeyRelease(long ptr) {
    }
    public void handleExposeEvent(int type, long ptr) {
    }
    /**
     * Activate automatic grab on first ButtonPress, 
     * deactivate on full mouse release
     */
    public void handleButtonPressRelease(int type, long ptr) {
        XButtonEvent xbe = new XButtonEvent(ptr);
        final int buttonState = xbe.get_state() & (Button1Mask | Button2Mask 
            | Button3Mask | Button4Mask | Button5Mask);
        switch (xbe.get_type()) {
        case ButtonPress:
            if (buttonState == 0) {
                XAwtState.setAutoGrabWindow(this);
            }
            break;
        case ButtonRelease:
            if (isFullRelease(buttonState, xbe.get_button())) {
                XAwtState.setAutoGrabWindow(null);
            }
            break;
        }               
    }
    public void handleMotionNotify(long ptr) {
    }
    public void handleXCrossingEvent(long ptr) {
    }
    public void handleConfigureNotifyEvent(long ptr) {
        XConfigureEvent xe = new XConfigureEvent(ptr);
        x = xe.get_x();
        y = xe.get_y();
        width = xe.get_width();
        height = xe.get_height();
    }
    /**
     * Checks ButtonRelease released all Mouse buttons
     */ 
    static boolean isFullRelease(int buttonState, int button) {
        switch (button) {
        case Button1:
            return buttonState == Button1Mask;
        case Button2:
            return buttonState == Button2Mask;
        case Button3:
            return buttonState == Button3Mask;
        case Button4:
            return buttonState == Button4Mask;
        case Button5:
            return buttonState == Button5Mask;
        }
        return buttonState == 0;
    }

    static boolean isGrabbedEvent(XAnyEvent ev) {
        switch (ev.get_type()) {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case KeyPress:
        case KeyRelease:
             return true;
        default:
             return false;
        }        
    }
    /**
     * Dispatches event to the grab Window or event ource window depending 
     * if the grab is active and the event type should be grabbed
     */
    static void dispatchToWindow(XAnyEvent ev) {
        XBaseWindow target = XAwtState.getGrabWindow();
        if (target == null || !isGrabbedEvent(ev)) {
            target = XToolkit.windowToXWindow(ev.get_window());
        }
        if (target != null) {
            target.dispatchEvent(ev);
        }
    }
                
    public void dispatchEvent(XAnyEvent ev) {
        if (eventLog.isLoggable(Level.FINER)) eventLog.finer(ev.toString());
        int type = ev.get_type();  
        long ptr = ev.pData;

        if (isDisposed()) {
            return;
        }

        switch (type)
        {
          case VisibilityNotify:
              handleVisibilityEvent(ptr);
              break;
          case ClientMessage:
              handleClientMessage(ptr);
              break;
          case Expose :
          case GraphicsExpose :  
              handleExposeEvent(type, ptr);
              break;
          case ButtonPress:
          case ButtonRelease:
              handleButtonPressRelease(type, ptr);
              break;

          case MotionNotify:
              handleMotionNotify(ptr);
              break;            
          case KeyPress:
              handleKeyPress(ptr);
              break;
          case KeyRelease:
              handleKeyRelease(ptr);
              break;
          case EnterNotify:
          case LeaveNotify:
              handleXCrossingEvent(ptr);
              break;
          case ConfigureNotify:
              handleConfigureNotifyEvent(ptr);
              break;
          case MapNotify:
              handleMapNotifyEvent(ptr);
              break;
          case UnmapNotify:
              handleUnmapNotifyEvent(ptr);
              break;
          case ReparentNotify:
              handleReparentNotifyEvent(ptr);
              break;
          case PropertyNotify:
              handlePropertyNotify(ptr);
              break;
          case DestroyNotify:
              handleDestroyNotify(ptr);
              break;
        }
    }
    protected boolean isEventDisabled(IXAnyEvent e) {
        return false;
    }

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

    void setDisposed(boolean d) {
        disposed = d;
    }

    boolean isDisposed() {
        return disposed;
    }
}
