/*
 * @(#)XWindow.java	1.122 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import java.awt.peer.ComponentPeer;
import sun.misc.Unsafe;
import sun.awt.*;
import java.awt.image.ColorModel;
import sun.java2d.SunGraphics2D;
import sun.awt.motif.X11FontMetrics;
import sun.awt.*;
import java.lang.ref.WeakReference;
import java.util.logging.*;
import java.lang.reflect.*;
import sun.java2d.SurfaceData;

public class XWindow extends XBaseWindow implements X11ComponentPeer {
    private static Logger log = Logger.getLogger("sun.awt.X11.XWindow");
    private static Logger insLog = Logger.getLogger("sun.awt.X11.insets.XWindow");
    private static Logger eventLog = Logger.getLogger("sun.awt.X11.event.XWindow");
    private static final Logger focusLog = Logger.getLogger("sun.awt.X11.focus.XWindow");

    // ButtonXXX events stuff
    static int rbutton = 0;
    static int lastX = 0, lastY = 0;
    static long lastTime = 0;
    static long lastButton = 0;
    static WeakReference lastWindowRef = null;
    static int clickCount = 0;

    // used to check if we need to re-create surfaceData.    
    int		oldWidth = -1;
    int 	oldHeight = -1;


    protected X11GraphicsConfig graphicsConfig;
    protected AwtGraphicsConfigData graphicsConfigData;

    boolean reparented;

    XWindow parent;

    Component target;

    private static int JAWT_LOCK_ERROR=0x00000001;
    private static int JAWT_LOCK_CLIP_CHANGED=0x00000002;
    private static int JAWT_LOCK_BOUNDS_CHANGED=0x00000004;
    private static int JAWT_LOCK_SURFACE_CHANGED=0x00000008;
    private int drawState = JAWT_LOCK_CLIP_CHANGED |
    JAWT_LOCK_BOUNDS_CHANGED |
    JAWT_LOCK_SURFACE_CHANGED;

    public static final String TARGET = "target",
        REPARENTED = "reparented"; // whether it is reparented by default

    SurfaceData surfaceData;

    XRepaintArea paintArea;
    // fallback default font object
    final static Font defaultFont = new Font("Dialog", Font.PLAIN, 12);


    native int getNativeColor(Color clr, GraphicsConfiguration gc);
    native void getWMInsets(long window, long left, long top, long right, long bottom, long border);
    native long getTopWindow(long window, long rootWin);
    native void getWindowBounds(long window, long x, long y, long width, long height);
    private native static void initIDs();

    private static Field isPostedField;
    static {
        initIDs();   
    }

    XWindow(XCreateWindowParams params) {
        super(params);
    }
   
    XWindow() {
    }

    XWindow(long parentWindow, Rectangle bounds) {
        super(new XCreateWindowParams(new Object[] {
            BOUNDS, bounds,
            PARENT_WINDOW, new Long(parentWindow)}));
    }

    XWindow(Component target, long parentWindow, Rectangle bounds) {
        super(new XCreateWindowParams(new Object[] {
            BOUNDS, bounds,
            PARENT_WINDOW, new Long(parentWindow),
            TARGET, target}));
    }

    XWindow(Component target, long parentWindow) {
        this(target, parentWindow, target.getBounds());
    }

    XWindow(Component target) {
        this(target, (target.getParent() == null) ? 0 : getParentWindow(target), target.getBounds());
    }

    XWindow(Object target) {
        this(null, 0, null);
    }

    /* This create is used by the XEmbeddedFramePeer since it has to create the window
       as a child of the netscape window. This netscape window is passed in as wid */
    XWindow(long parentWindow) {
        super(new XCreateWindowParams(new Object[] {
            PARENT_WINDOW, new Long(parentWindow),
            REPARENTED, Boolean.TRUE,
            EMBEDDED, Boolean.TRUE}));
    }

    protected void initGraphicsConfiguration() {
        graphicsConfig = (X11GraphicsConfig) target.getGraphicsConfiguration();
        graphicsConfigData = new AwtGraphicsConfigData(graphicsConfig.getAData());
    }

    void preInit(XCreateWindowParams params) {
        super.preInit(params);
        reparented = Boolean.TRUE.equals(params.get(REPARENTED));

        target = (Component)params.get(TARGET);

        initGraphicsConfiguration();

        AwtGraphicsConfigData gData = getGraphicsConfigurationData();
        X11GraphicsConfig config = (X11GraphicsConfig) getGraphicsConfiguration();
        XVisualInfo visInfo = gData.get_awt_visInfo();
        params.putIfNull(EVENT_MASK, KeyPressMask | KeyReleaseMask
            | FocusChangeMask | ButtonPressMask | ButtonReleaseMask 
            | EnterWindowMask | LeaveWindowMask | PointerMotionMask 
            | ButtonMotionMask | ExposureMask | StructureNotifyMask);
            
        if (target != null) {
            params.putIfNull(BOUNDS, target.getBounds());
        } else {
            params.putIfNull(BOUNDS, new Rectangle(0, 0, MIN_SIZE, MIN_SIZE));
        }
        params.putIfNull(BORDER_PIXEL, new Long(0));
        getColorModel(); // fix 4948833: this call forces the color map to be initialized
        params.putIfNull(COLORMAP, gData.get_awt_cmap());
        params.putIfNull(DEPTH, gData.get_awt_depth());
        params.putIfNull(VISUAL_CLASS, new Integer((int)XlibWrapper.InputOutput));
        params.putIfNull(VISUAL, visInfo.get_visual());
        params.putIfNull(VALUE_MASK, XlibWrapper.CWBorderPixel | XlibWrapper.CWEventMask | XlibWrapper.CWColormap);
        Long parentWindow = (Long)params.get(PARENT_WINDOW);
        if (parentWindow == null || parentWindow.longValue() == 0) {
            if (visInfo != null) {
                synchronized(getAWTLock()) {
                    int screen = visInfo.get_screen();
                    if (screen != -1) {
                        params.add(PARENT_WINDOW, XlibWrapper.RootWindow(XToolkit.getDisplay(), screen));
                    } else {
                        params.add(PARENT_WINDOW, XToolkit.getDefaultRootWindow());
                    }
                }
            }
        }
        
        paintArea = new XRepaintArea();
        if (target != null) {
            this.parent = getParentXWindowObject(target.getParent());
        }
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        
        setWMClass(getWMClass());

        surfaceData = graphicsConfig.createSurfaceData(this);
        Color c;
        if (target != null && (c = target.getBackground()) != null) {
            // We need a version of setBackground that does not call repaint !!
            // and one that does not get overridden. The problem is that in postInit
            // we call setBackground and we dont have all the stuff initialized to
            // do a full paint for most peers. So we cannot call setBackground in postInit.
            // instead we need to call xSetBackground.
            xSetBackground(c);
        }
    }

    public GraphicsConfiguration getGraphicsConfiguration() {
        if (graphicsConfig == null) {
            initGraphicsConfiguration();
        }
        return graphicsConfig;
    }

    public AwtGraphicsConfigData getGraphicsConfigurationData() {
        if (graphicsConfigData == null) {
            initGraphicsConfiguration();
        }
        return graphicsConfigData;
    }

    protected String[] getWMClass() {
        return new String[] {XToolkit.getCorrectXIDString(getClass().getName()), XToolkit.getAWTAppClassName()};
    }

    static long getParentWindow(Component target) {

        ComponentPeer peer = target.getParent().getPeer();
        Component temp = target.getParent();
        while (!(peer instanceof XWindow))
        {
            temp = temp.getParent();  
            peer = temp.getPeer();
        }
 
        if (peer != null && peer instanceof XWindow)
            return ((XWindow)peer).getContentWindow(); 
        else return 0;
    }


    static XWindow getParentXWindowObject(Component target) {
        if (target == null) return null;   
        Component temp = target.getParent();
        if (temp == null) return null;
        ComponentPeer peer = temp.getPeer();
        if (peer == null) return null;
        while ((peer != null) && !(peer instanceof XWindow))
        {
            temp = temp.getParent();  
            peer = temp.getPeer();
        }
        if (peer != null && peer instanceof XWindow)
            return (XWindow) peer;
        else return null;
    }

    
    boolean isParentOf(XWindow win) {
        if (!(target instanceof Container) || win == null || win.getTarget() == null) {
            return false;
        }
        Container parent = ComponentAccessor.getParent_NoClientCode(win.target);
        while (parent != null && parent != target) {
            parent = ComponentAccessor.getParent_NoClientCode(parent);
        }
        return (parent == target);
    }

    public Object getTarget() {
        return target;
    }
    public Component getEventSource() {
        return target;
    }

    public ColorModel getColorModel(int transparency) {
        return graphicsConfig.getColorModel (transparency);
    }

    public ColorModel getColorModel() {
        if (graphicsConfig != null) {
            return graphicsConfig.getColorModel ();
        }
        else {
            return XToolkit.getStaticColorModel();
        }
    }

    Graphics getGraphics(SurfaceData surfData, Color afore, Color aback, Font afont) {
        if (surfData == null) return null;

        Component target = (Component) this.target;

        /* Fix for bug 4746122. Color and Font shouldn't be null */
        Color bgColor = aback;
        if (bgColor == null) {
            bgColor = SystemColor.window;
        }
        Color fgColor = afore;
        if (fgColor == null) {
            fgColor = SystemColor.windowText;
        }
        Font font = afont; 
        if (font == null) {
            font = defaultFont;
        }
        return new SunGraphics2D(surfData, fgColor, bgColor, font);        
    }

    public Graphics getGraphics() {
        return getGraphics(surfaceData,
                           target.getForeground(),
                           target.getBackground(),
                           target.getFont());
    }

    public FontMetrics getFontMetrics(Font font) {
        return Toolkit.getDefaultToolkit().getFontMetrics(font);
    }

    public Rectangle getTargetBounds() {
        return target.getBounds();
    }

    void prePostEvent(AWTEvent e) {
    }

    static Method m_sendMessage;
    static void sendEvent(final AWTEvent e) {
        if (isPostedField == null) {
            isPostedField = XToolkit.getField(AWTEvent.class, "isPosted");
        }
        PeerEvent pe = new PeerEvent(Toolkit.getDefaultToolkit(), new Runnable() {
                public void run() {
                    try {
                        isPostedField.setBoolean(e, true);
                    } catch (IllegalArgumentException e) {
                        assert(false);
                    } catch (IllegalAccessException e) {
                        assert(false);
                    }
                    ((Component)e.getSource()).dispatchEvent(e);
                }
            }, PeerEvent.ULTIMATE_PRIORITY_EVENT);
        if (focusLog.isLoggable(Level.FINER) && (e instanceof FocusEvent)) focusLog.finer("Sending " + e);
        XToolkit.postEvent(XToolkit.targetToAppContext(e.getSource()), pe);
    }


/*
 * Post an event to the event queue.
 */
// NOTE: This method may be called by privileged threads.
//       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    void postEvent(AWTEvent event) {
        XToolkit.postEvent(XToolkit.targetToAppContext(event.getSource()), event);
    }

    static void postEventStatic(AWTEvent event) {
        XToolkit.postEvent(XToolkit.targetToAppContext(event.getSource()), event);
    }

    public void postEventToEventQueue(final AWTEvent event) {
        prePostEvent(event);
        postEvent(event);
    }

    // We need a version of setBackground that does not call repaint !!
    // and one that does not get overridden. The problem is that in postInit
    // we call setBackground and we dont have all the stuff initialized to
    // do a full paint for most peers. So we cannot call setBackground in postInit.
    final public void xSetBackground(Color c) {
        int pixel=0;
        try {
            XToolkit.awtLock();
            winBackground(c);
            pixel = surfaceData.pixelFor(c.getRGB());
            XlibWrapper.XSetWindowBackground(XToolkit.getDisplay(), getContentWindow(), pixel);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    public void setBackground(Color c) {
        xSetBackground(c);
    }
    
    Color backgroundColor;
    void winBackground(Color c) {
        backgroundColor = c;
    }

    public Color getWinBackground() {
        Color c = null;

        if (backgroundColor != null) {
            c = backgroundColor;
        } else if (parent != null) {
            c = parent.getWinBackground();
        }

        if (c instanceof SystemColor) {
            c = new Color(c.getRGB());
        }

        return c;
    }

    public boolean isEmbedded() {
        return embedded;
    }

    public  void repaint(int x,int y, int width, int height) {
        if (!isVisible()) {
            return;
        }
        Graphics g = getGraphics();        
        if (g != null) {
            try {
                g.setClip(x,y,width,height); 
                paint(g);
            } finally {
                g.dispose();
            }
        }
    }

    public  void repaint() {
        if (!isVisible()) {
            return;
        }
        Graphics g = getGraphics();
        if (g != null) {
            try {
                paint(g);                
            } finally {
                g.dispose();
            }
        }
    }

    void paint(Graphics g) {
    }

    public void popup(int x, int y, int width, int height) {
        // TBD: grab the pointer
        xSetBounds(x, y, width, height);
    }

    public void handleExposeEvent(int type, long ptr) {
        super.handleExposeEvent(type, ptr);
        XExposeEvent xe = new XExposeEvent(ptr);    
        if (isEventDisabled(xe)) {
            return;
        }
        int x = xe.get_x();
        int y = xe.get_y();
        int w = xe.get_width();
        int h = xe.get_height();

        Component target = (Component)getEventSource();

        if (!ComponentAccessor.getIgnoreRepaint(target)
            && ComponentAccessor.getWidth(target) != 0
            && ComponentAccessor.getHeight(target) != 0)
        {
            PaintEvent pe = new PaintEvent(target, PaintEvent.PAINT, 
                                           new Rectangle(x, y, w, h));
            postEventToEventQueue(pe);
        }

        
    }

    static int getModifiers(int state, int button, int keyCode) {
        int modifiers = 0;

        if (((state & XlibWrapper.ShiftMask) != 0) ^ (keyCode == KeyEvent.VK_SHIFT)) {
            modifiers |= InputEvent.SHIFT_DOWN_MASK;
        }
        if (((state & XlibWrapper.ControlMask) != 0) ^ (keyCode == KeyEvent.VK_CONTROL)) {
            modifiers |= InputEvent.CTRL_DOWN_MASK;
        }
        if (((state & XToolkit.metaMask) != 0) ^ (keyCode == KeyEvent.VK_META)) {
            modifiers |= InputEvent.META_DOWN_MASK;
        }
        if (((state & XToolkit.altMask) != 0) ^ (keyCode == KeyEvent.VK_ALT)) {
            modifiers |= InputEvent.ALT_DOWN_MASK;
        }
        if (((state & XToolkit.modeSwitchMask) != 0) ^ (keyCode == KeyEvent.VK_ALT_GRAPH)) {
            modifiers |= InputEvent.ALT_GRAPH_DOWN_MASK;
        }
        if (((state & XlibWrapper.Button1Mask) != 0) ^ (button == MouseEvent.BUTTON1)) {
            modifiers |= InputEvent.BUTTON1_DOWN_MASK;
        }
        if (((state & XlibWrapper.Button2Mask) != 0) ^ (button == MouseEvent.BUTTON2)) {
            modifiers |= InputEvent.BUTTON2_DOWN_MASK;
        }
        if (((state & XlibWrapper.Button3Mask) != 0) ^ (button == MouseEvent.BUTTON3)) {
            modifiers |= InputEvent.BUTTON3_DOWN_MASK;
        }
        return modifiers;
    }

    /**
     * Returns true if this event is disabled and shouldn't be passed to Java.
     * Default implementation returns false for all events.
     */
    static int getRightButtonNumber() {
        if (rbutton == 0) { // not initialized yet
            try { 
                XToolkit.awtLock();
                rbutton = XlibWrapper.XGetPointerMapping(XToolkit.getDisplay(), XlibWrapper.ibuffer, 3);
            }
            finally {
                XToolkit.awtUnlock();
            }
        }
        return rbutton;
    }

    public void handleButtonPressRelease(int type, long ptr) {
        super.handleButtonPressRelease(type, ptr);
        XButtonEvent xbe = new XButtonEvent(ptr);
        if (isEventDisabled(xbe)) {
            return;
        }
        if (eventLog.isLoggable(Level.FINE)) eventLog.fine(xbe.toString());
        long when; 
        int modifiers; 
        boolean popupTrigger = false;
        int button=0;
        boolean mouseClicked = false;
        boolean wheel_mouse = false;
        long lbutton = xbe.get_button();
        when = xbe.get_time();
        long jWhen = XToolkit.nowMillisUTC_offset(when);
        
        int x = xbe.get_x();
        int y = xbe.get_y(); 
        if (xbe.get_window() != window) {
            Point localXY = toLocal(xbe.get_x_root(), xbe.get_y_root());
            x = localXY.x;
            y = localXY.y;
        }

        if (xbe.get_type() == XlibWrapper.ButtonPress) {
            XWindow lastWindow = (lastWindowRef != null) ? ((XWindow)lastWindowRef.get()):(null);
            /* 
               multiclick checking 
            */
            if (eventLog.isLoggable(Level.FINEST)) eventLog.finest("lastWindow = " + lastWindow + ", lastButton " 
                                                                   + lastButton + ", lastTime " + lastTime + ", multiClickTime " 
                                                                   + XToolkit.getMultiClickTime());
            if (lastWindow == this && lastButton == lbutton && (when - lastTime) < XToolkit.getMultiClickTime()) {
                clickCount++;
            } else {
                clickCount = 1;
                lastWindowRef = new WeakReference(this);
                lastButton = lbutton;
                lastX = x;
                lastY = y;
            }
            lastTime = when;


            /* 
               Check for popup trigger !!
            */
            if (lbutton == getRightButtonNumber() || lbutton > 2) {
                popupTrigger = true; 
            } else {
                popupTrigger = false;
            }
        } else {
            XWindow lastWindow = (lastWindowRef != null) ? ((XWindow)lastWindowRef.get()):(null);
            if (lastWindow == this) {
                mouseClicked = true;
            }
        }

        if (lbutton == XlibWrapper.Button1)
            button = MouseEvent.BUTTON1;
        else if (lbutton ==  XlibWrapper.Button2 )
            button = MouseEvent.BUTTON2;
        else if (lbutton == XlibWrapper.Button3)
            button = MouseEvent.BUTTON3;
        else if (lbutton == XlibWrapper.Button4) { 
            button = 4;
            wheel_mouse = true;
        } else if (lbutton == XlibWrapper.Button5) {
            button = 5;
            wheel_mouse = true;
        }

        modifiers = getModifiers(xbe.get_state(),button,0);

        if (!wheel_mouse) { 
            MouseEvent me = new MouseEvent((Component)getEventSource(),
                                           type == XlibWrapper.ButtonPress ? MouseEvent.MOUSE_PRESSED : MouseEvent.MOUSE_RELEASED,
                                           jWhen,modifiers, x, y,clickCount,popupTrigger,button);

            postEventToEventQueue(me);
            if (mouseClicked) {
                postEventToEventQueue(me = new MouseEvent((Component)getEventSource(),
                                                     MouseEvent.MOUSE_CLICKED,
                                                     jWhen,
                                                     modifiers,
                                                     x, y,
                                                     clickCount,
                                                     false, button));
            }

        }
        else {
            if (xbe.get_type() == XlibWrapper.ButtonPress) {
                MouseWheelEvent mwe = new MouseWheelEvent((Component)getEventSource(),MouseEvent.MOUSE_WHEEL, jWhen, 
                                                          modifiers, x, y,clickCount,false,MouseWheelEvent.WHEEL_UNIT_SCROLL,
                                                          3,button==4 ?  -1 : 1);
                postEventToEventQueue(mwe);
            }
        }
    }

    public void handleMotionNotify(long ptr) {
        super.handleMotionNotify(ptr);
        XMotionEvent xme = new XMotionEvent(ptr);
        if (isEventDisabled(xme)) {
            return;
        }
        /* To be done :
           add multiclick checking 
        */
        clickCount = 0;
        //lastWindowRef = null;

        long jWhen = XToolkit.nowMillisUTC_offset(xme.get_time());
        int modifiers = getModifiers(xme.get_state(),0,0);
        boolean popupTrigger = false; 
        boolean dragging = 
            (xme.get_state() & (Button1Mask | Button2Mask | Button3Mask)) != 0;
        int type = dragging ? MouseEvent.MOUSE_DRAGGED : 
            MouseEvent.MOUSE_MOVED;
        Component source = (Component)getEventSource();
        
        int x = xme.get_x();
        int y = xme.get_y();
        if (xme.get_window() != window) {
            Point localXY = toLocal(xme.get_x_root(), xme.get_y_root());
            x = localXY.x;
            y = localXY.y;
        }
        
        MouseEvent mme = new MouseEvent(source, type, jWhen, modifiers, x, y,
            clickCount, popupTrigger, MouseEvent.NOBUTTON);

        postEventToEventQueue(mme);
    }


    // REMIND: need to implement looking for disabled events
    public native int nativeHandleKeyEvent(Component target,int keyid,long ptr);
    public native int nativeGetKeyCode(Component target,int keyid,long ptr);

    public void handleXCrossingEvent(long ptr) {
        super.handleXCrossingEvent(ptr);
        XCrossingEvent xce = new XCrossingEvent(ptr);

        if (eventLog.isLoggable(Level.FINE)) eventLog.fine(xce.toString());        

        // Skip event If it was caused by a grab
        if (xce.get_mode() != NotifyNormal) {
            return;
        }

        // X sends XCrossing to all hierarchy so if the edge of child equals to
        // ancestor and mouse enters child, the ancestor will get an event too.
        // From java point the event is bogus as ancestor is obscured, so if
        // the child can get java event itself, we skip it on ancestor.
        long childWnd = xce.get_subwindow();
        if (childWnd != None) {
            XBaseWindow child = XToolkit.windowToXWindow(childWnd);
            if (child != null && child instanceof XWindow && 
                !child.isEventDisabled(xce)) 
            {
                return;
            }
        }

        // Remember old component with mouse to have the opportunity to send it MOUSE_EXITED.
        final Component compWithMouse = XAwtState.getComponentMouseEntered();

        if (xce.get_type() == EnterNotify) {
            // Change XAwtState's component mouse entered to the up-to-date one before requesting
            // to update the cursor since XAwtState.getComponentMouseEntered() is used when the
            // cursor is updated (in XGlobalCursorManager.findHeavyweightUnderCursor()).
            XAwtState.setComponentMouseEntered(getEventSource());
            XGlobalCursorManager.nativeUpdateCursor(getEventSource());
        } else { // LeaveNotify:
            XAwtState.setComponentMouseEntered(null);
        }

        if (isEventDisabled(xce)) {
            return;
        }

        long jWhen = XToolkit.nowMillisUTC_offset(xce.get_time());
        int modifiers = getModifiers(xce.get_state(),0,0);
        int clickCount = 0;
        boolean popupTrigger = false; 
        int x = xce.get_x();
        int y = xce.get_y();
        if (xce.get_window() != window) {
            Point localXY = toLocal(xce.get_x_root(), xce.get_y_root());
            x = localXY.x;
            y = localXY.y;
        }

        // This code tracks boundary crossing and ensures MOUSE_ENTER/EXIT
        // are posted in alternate pairs
        if (compWithMouse != null) {
            MouseEvent me = new MouseEvent(compWithMouse, 
                MouseEvent.MOUSE_EXITED, jWhen, modifiers, xce.get_x(), 
                xce.get_y(), clickCount, popupTrigger, 
                MouseEvent.NOBUTTON);
            postEventToEventQueue(me);
            eventLog.finest("Clearing last window ref");
            lastWindowRef = null;
        }
        if (xce.get_type() == EnterNotify) {
            MouseEvent me = new MouseEvent(getEventSource(), MouseEvent.MOUSE_ENTERED, 
                jWhen, modifiers, xce.get_x(), xce.get_y(), clickCount, 
                popupTrigger, MouseEvent.NOBUTTON);
            postEventToEventQueue(me);            
        }
    }

    public void doLayout(int x, int y, int width, int height) {}

    public void handleConfigureNotifyEvent(long ptr) {
        Rectangle oldBounds = getBounds();

        super.handleConfigureNotifyEvent(ptr);
        XConfigureEvent xe = new XConfigureEvent(ptr);
        if (isEventDisabled(xe)) {
            return;
        }
        long eventWindow = xe.get_window();

        ComponentEvent ce;
  
//  if ( Check if it's a resize, a move, or a stacking order change )
//  {
        Rectangle bounds = getBounds();
        if (!bounds.getSize().equals(oldBounds.getSize())) {
            postEventToEventQueue(new ComponentEvent(getEventSource(), ComponentEvent.COMPONENT_RESIZED));
        }
        if (!bounds.getLocation().equals(oldBounds.getLocation())) {
            postEventToEventQueue(new ComponentEvent(getEventSource(), ComponentEvent.COMPONENT_MOVED));
        }  
//  }
    }

    public void handleMapNotifyEvent(long ptr) {
        super.handleMapNotifyEvent(ptr);
        log.log(Level.FINE, "Mapped {0}", new Object[] {this});
        XMapEvent xe = new XMapEvent(ptr);
        if (isEventDisabled(xe)) {
            return;
        }
        ComponentEvent ce;
  
        ce = new ComponentEvent(getEventSource(), ComponentEvent.COMPONENT_SHOWN);
        postEventToEventQueue(ce);
    }

    public void handleUnmapNotifyEvent(long ptr) {
        super.handleUnmapNotifyEvent(ptr);
        XUnmapEvent xe = new XUnmapEvent(ptr);
        if (isEventDisabled(xe)) {
            return;
        }
        ComponentEvent ce;
  
        ce = new ComponentEvent(target, ComponentEvent.COMPONENT_HIDDEN);
        postEventToEventQueue(ce);
    }
    public void handleKeyPress(long ptr) {
        super.handleKeyPress(ptr);
        XKeyEvent ev = new XKeyEvent(ptr);
        if (eventLog.isLoggable(Level.FINE)) eventLog.fine(ev.toString());
        if (isEventDisabled(ev)) {
            return;
        }
        final Component currentSource = (Component)getEventSource();
        if ((nativeHandleKeyEvent(currentSource,KeyEvent.KEY_PRESSED,ev.pData)) == KeyEvent.VK_F10) {
            XToolkit.executeOnEventHandlerThread(currentSource, new Runnable() {
                    public void run() {
                        handleF10onEDT(currentSource);
                    }
                }
            );
        }
    }

    void handleF10onEDT(Component source) {
        XFramePeer framePeer = XToolkit.getParentFramePeer(source);
        if (framePeer != null) {
            XMenuBarPeer mPeer = framePeer.getMenubarPeer();
            if (mPeer != null) {
                mPeer.handleF10KeyPress();
            }
        }
    }        

    public void handleKeyRelease(long ptr) {
        super.handleKeyRelease(ptr);
        XKeyEvent ev = new XKeyEvent(ptr);
        if (eventLog.isLoggable(Level.FINE)) eventLog.fine(ev.toString());
        if (isEventDisabled(ev)) {
            return;
        }
        nativeHandleKeyEvent((Component)getEventSource(),KeyEvent.KEY_RELEASED,ptr); // handle it natively for now !!, FIXME
    }

    public void reshape(Rectangle bounds) {
        reshape(bounds.x, bounds.y, bounds.width, bounds.height);
    }

    public void reshape(int x, int y, int width, int height) {
        if (width <= 0) {
            width = 1;
        }
        if (height <= 0) {
            height = 1;
        }
	this.x = x;
	this.y = y;
	this.width = width;
	this.height = height;
        xSetBounds(x, y, width, height);
    }

    boolean isShowing() {
        return visible;
    }

    public String toString() {
        return super.toString() + "(" + Long.toString(getWindow(), 16) + ")";
    }

    boolean isResizable() {
        return true;
    }

    void updateSizeHints() {
        updateSizeHints(x, y, width, height);
    }

    void updateSizeHints(int x, int y, int width, int height) {
        long flags = XlibWrapper.PSize | XlibWrapper.PPosition | XlibWrapper.USPosition;
        if (!isResizable()) {
            log.log(Level.FINER, "Window {0} is not resizable", new Object[] {this});
            flags |= XlibWrapper.PMinSize | XlibWrapper.PMaxSize;
        } else {
            log.log(Level.FINER, "Window {0} is resizable", new Object[] {this});
        }
        setSizeHints(flags, x, y, width, height);
    }

    void updateSizeHints(int x, int y) {
        long flags = XlibWrapper.PPosition | XlibWrapper.USPosition;
        if (!isResizable()) {
            log.log(Level.FINER, "Window {0} is not resizable", new Object[] {this});
            flags |= XlibWrapper.PMinSize | XlibWrapper.PMaxSize | XlibWrapper.PSize;
        } else {
            log.log(Level.FINER, "Window {0} is resizable", new Object[] {this});
        }
        setSizeHints(flags, x, y, width, height);
    }

      void validateSurface() {
        if ((width != oldWidth) || (height != oldHeight)) {
            SurfaceData oldData = surfaceData;
            if (oldData != null) {
                surfaceData = graphicsConfig.createSurfaceData(this);
                oldData.invalidate();
            }
            oldWidth = width;
            oldHeight = height;
        }
    }

    public SurfaceData getSurfaceData() {
        return surfaceData;
    }

    public void dispose() {
        SurfaceData oldData = surfaceData;
        surfaceData = null;
        if (oldData != null) {
            oldData.invalidate();
        }
        XToolkit.targetDisposedPeer(target, this);
        destroy();
    }

    public Point getLocationOnScreen() {
	synchronized (target.getTreeLock()) {
	    Component comp = target;

	    while (comp != null && !(comp instanceof Window)) {
		comp = ComponentAccessor.getParent_NoClientCode(comp);
	    }

	    // applets, embedded, etc - translate directly
	    // XXX: override in subclass?
	    if (comp == null || comp instanceof sun.awt.EmbeddedFrame) {
		return toGlobal(0, 0);
	    }
  	   
            synchronized (getAWTLock()) {
 		Object wpeer = XToolkit.targetToPeer(comp);
 		if (wpeer == null
 		    || !(wpeer instanceof XDecoratedPeer)
 		    || ((XDecoratedPeer)wpeer).configure_seen)
 	        {
  		    return toGlobal(0, 0);
  		}
  
 		// wpeer is an XDecoratedPeer not yet fully adopted by WM
 		Point pt = toOtherWindow(getContentWindow(), 
 					 ((XDecoratedPeer)wpeer).getContentWindow(),
  					 0, 0);

		pt.x += comp.getX();
		pt.y += comp.getY();
		return pt;
	    }
	}
    }

    public void postKeyEvent(int id, long when, int keyCode, char keyChar,
        int keyLocation, int state)
    {
        long jWhen = XToolkit.nowMillisUTC_offset(when);
        int modifiers = getModifiers(state, 0, keyCode);
        KeyEvent ke = new KeyEvent((Component)getEventSource(), id, jWhen,
                                   modifiers, keyCode, keyChar, keyLocation);
        postEventToEventQueue(ke);
    }
}
