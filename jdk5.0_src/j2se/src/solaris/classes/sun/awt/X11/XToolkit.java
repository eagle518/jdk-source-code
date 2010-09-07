/*
 * @(#)XToolkit.java	1.110 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import java.awt.peer.*;
import sun.awt.*;
import sun.awt.SunToolkit;
import java.util.ArrayList;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Collection;
import java.util.Iterator;
import java.util.Vector;
import java.awt.dnd.DragSource;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.MouseDragGestureRecognizer;
import java.awt.dnd.InvalidDnDOperationException;
import java.awt.dnd.peer.DragSourceContextPeer;
import java.util.Properties;
import java.util.Map;
import java.awt.image.*;
import java.security.*;
import java.awt.im.InputMethodHighlight;
import java.awt.im.spi.InputMethodDescriptor;
import java.awt.datatransfer.Clipboard;
import javax.swing.LookAndFeel;
import javax.swing.UIDefaults;
import java.io.*;
import java.util.logging.*;
import sun.misc.PerformanceLogger;
import sun.print.PrintJob2D;
import java.lang.reflect.*;

public class XToolkit extends UNIXToolkit implements Runnable, XConstants {
    private static Logger log = Logger.getLogger("sun.awt.X11.XToolkit");
    private static Logger eventLog = Logger.getLogger("sun.awt.X11.event.XToolkit");
    private static final Logger timeoutTaskLog = Logger.getLogger("sun.awt.X11.timeoutTask.XToolkit");

    static final boolean PRIMARY_LOOP = false;
    static final boolean SECONDARY_LOOP = true;

    private static String awtAppClassName = null;

    // the system clipboard - CLIPBOARD selection
    XClipboard clipboard;
    // the system selection - PRIMARY selection
    XClipboard selection;
 
    // Dynamic Layout Resize client code setting
    protected static boolean dynamicLayoutSetting = false;

    /**
     * True when the x settings have been loaded.
     */
    private boolean loadedXSettings;

    /**
     * XSETTINGS for the default screen.
     * <p>
     */
    private XSettings xs;

    static int arrowCursor;
    static TreeMap winMap = new TreeMap();
    static HashMap specialPeerMap = new HashMap();
    static HashMap winToDispatcher = new HashMap();
    static long display;
    static UIDefaults uidefaults;
    static X11GraphicsEnvironment localEnv;   
    static X11GraphicsDevice device;
    static final X11GraphicsConfig config;
    static long tempptr = XlibWrapper.unsafe.allocateMemory(4096);
    static int awt_multiclick_time;
    static boolean securityWarningEnabled;
    
    private static int screenWidth = -1, screenHeight = -1; // Dimensions of default screen
    static long awt_defaultFg; // Pixel
    private static XMouseInfoPeer xPeer;
    private static Method m_removeSourceEvents;

    static {
//      System.loadLibrary("mawt");    
        initSecurityWarning();
        if (GraphicsEnvironment.isHeadless()) {
            config = null;
        } else {
            localEnv = (X11GraphicsEnvironment) GraphicsEnvironment
                .getLocalGraphicsEnvironment();
            device = (X11GraphicsDevice) localEnv.getDefaultScreenDevice();
            config = (X11GraphicsConfig) (device.getDefaultConfiguration());
            if (device != null) {
                display = device.getDisplay();
            }
            setupModifierMap();
            initIDs();
        }
        m_removeSourceEvents = XToolkit.getMethod(EventQueue.class, "removeSourceEvents", new Class[] {Object.class, Boolean.TYPE}) ;
    }

    // Error handler stuff
    static XErrorEvent saved_error;
    static long saved_error_handler;
    static XErrorHandler curErrorHandler;
    // Should be called under LOCK, before releasing LOCK RESTORE_XERROR_HANDLER should be called
    static void WITH_XERROR_HANDLER(XErrorHandler handler) {
        saved_error = null;
        curErrorHandler = handler;
        XSync();
        saved_error_handler = XlibWrapper.SetToolkitErrorHandler();
    }
    static void XERROR_SAVE(XErrorEvent event) {
        saved_error = event;
    }
    // Should be called under LOCK
    static void RESTORE_XERROR_HANDLER() {
        XSync();
        XlibWrapper.XSetErrorHandler(saved_error_handler);
        curErrorHandler = null;
    }
    // Should be called under LOCK
    static int SAVED_ERROR_HANDLER(long display, XErrorEvent error) {
        return XlibWrapper.CallErrorHandler(saved_error_handler, display, error.pData);
    }
    interface XErrorHandler {
        int handleError(long display, XErrorEvent err);
    }
    static int GlobalErrorHandler(long display, long event_ptr) {
        XErrorEvent event = new XErrorEvent(event_ptr);
        try {
            if (curErrorHandler != null) {
                return curErrorHandler.handleError(display, event);
            } else {
                return SAVED_ERROR_HANDLER(display, event);
            }
        } finally {
        }
    }

/*
 * Instead of validating window id, we simply call XGetWindowProperty,
 * but temporary install this function as the error handler to ignore
 * BadWindow error.
 */
    static XErrorHandler IgnoreBadWindowHandler = new XErrorHandler() {
            public int handleError(long display, XErrorEvent err) {
                XERROR_SAVE(err);
                if (err.get_error_code() == BadWindow) {
                    return 0;
                } else {
                    return SAVED_ERROR_HANDLER(display, err);
                }
            }
        };


    private native static void initIDs();
    native static void waitForEvents();
    static Thread toolkitThread;
    static boolean isToolkitThread() {
        return Thread.currentThread() == toolkitThread;
    }

    static void initSecurityWarning() {
        // Enable warning only for internal builds
        String runtime = getSystemProperty("java.runtime.version");
        securityWarningEnabled = (runtime != null && runtime.contains("internal"));
    }

    static boolean isSecurityWarningEnabled() {
        return securityWarningEnabled;
    }

    static boolean debug = false;

    static final void awtLock() {
 
        if (debug)
        {
            System.out.println(" --------   awtLock --------------");
            Thread.currentThread().dumpStack();
        }
        XlibWrapper.unsafe.monitorEnter(getAWTLock());
    }

    static final void  awtUnlock() {
 
        if (debug)
        {
            System.out.println(" --------   awtUnLock --------------");
            Thread.currentThread().dumpStack();
        }
        XlibWrapper.unsafe.monitorExit(getAWTLock());
    }

    static final Object getAWTLock() {
        return X11GraphicsEnvironment.class;
    }
    
    public native void nativeLoadSystemColors(int[] systemColors);

    static UIDefaults getUIDefaults() {
        if (uidefaults == null) {
            initUIDefaults();
        }
        return uidefaults;
    }
   
    public void loadSystemColors(int[] systemColors) {
        nativeLoadSystemColors(systemColors);
        MotifColorUtilities.loadSystemColors(systemColors);
    }

    
   
    static void initUIDefaults() {
        try {
            // Load Defaults from MotifLookAndFeel 
            
            // This dummy load is necessary to get SystemColor initialized. !!!!!!
            Color c = SystemColor.text;
            
            LookAndFeel lnf = new XAWTLookAndFeel();
            uidefaults = lnf.getDefaults();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    static Object displayLock = new Object();

    static native long xGetDisplay(); 

    static long getDisplay() {
        return display;
    }

    static long getDefaultRootWindow() {
        synchronized(getAWTLock()) {        
            long res = XlibWrapper.RootWindow(XToolkit.getDisplay(), 
                XlibWrapper.DefaultScreen(XToolkit.getDisplay()));
        
            if (res == 0) {
               throw new IllegalStateException("Root window must not be null");
            }
            return res;
        }
    }
    
    void init() {
        synchronized(getAWTLock()) {        
            XlibWrapper.XSupportsLocale();
            if (XlibWrapper.XSetLocaleModifiers("") == null) {
                log.finer("X locale modifiers are not supported, using default");
            }

            AwtScreenData defaultScreen = new AwtScreenData(XToolkit.getDefaultScreenData());
            awt_defaultFg = defaultScreen.get_blackpixel();

            arrowCursor = XlibWrapper.XCreateFontCursor(XToolkit.getDisplay(), 
                XCursorFontConstants.XC_arrow);
        }

        if (log.isLoggable(Level.FINE)) {
            Runtime.getRuntime().addShutdownHook(new Thread() {
                    public void run() {
                        dumpPeers();
                    }
                });
        }
    }

    static String getCorrectXIDString(String val) {
        if (val != null) {
            return val.replace('.', '-');
        } else {
            return val;
        }
    }

    static native String getEnv(String key);


    static String getAWTAppClassName() {
        return awtAppClassName;
    }

    static final String DATA_TRANSFERER_CLASS_NAME = "sun.awt.X11.XDataTransferer";

    public XToolkit() {
        super();
        if (PerformanceLogger.loggingEnabled()) {
            PerformanceLogger.setTime("XToolkit construction");
        }

        if (!GraphicsEnvironment.isHeadless()) {
            String mainClassName = null;
        
            StackTraceElement trace[] = (new Throwable()).getStackTrace();
            int bottom = trace.length - 1;
            if (bottom >= 0) {
                mainClassName = trace[bottom].getClassName();
            }
            if (mainClassName == null || mainClassName.equals("")) {
                mainClassName = "AWT";
            }
            awtAppClassName = getCorrectXIDString(mainClassName);

            init();
            XWM.init();
            SunToolkit.setDataTransfererClassName(DATA_TRANSFERER_CLASS_NAME);
            toolkitThread = new Thread(this, "AWT-XAWT");
            toolkitThread.setPriority(Thread.NORM_PRIORITY + 1);
            toolkitThread.setDaemon(true);
            ThreadGroup mainTG = (ThreadGroup)AccessController.doPrivileged(
                                                                            new PrivilegedAction() {
                                                                                    public Object run() {
                                                                                        ThreadGroup currentTG =
                                                                                            Thread.currentThread().getThreadGroup();
                                                                                        ThreadGroup parentTG = currentTG.getParent();
                                                                                        while (parentTG != null) {
                                                                                            currentTG = parentTG;
                                                                                            parentTG = currentTG.getParent();
                                                                                        }
                                                                                        return currentTG;
                                                                                    }
                                                                                });
            toolkitThread.start();
        }
    }

    public ButtonPeer createButton(Button target) {
        ButtonPeer peer = new XButtonPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public FramePeer createFrame(Frame target) {
        FramePeer peer = new XFramePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    static void addToWinMap(long window, XBaseWindow xwin)
    {
        synchronized(winMap) {
            winMap.put(new Long(window),xwin); 
        }
    }

    static void removeFromWinMap(long window, XBaseWindow xwin) {
        synchronized(winMap) {
            winMap.remove(new Long(window));
        }
    }
    static XBaseWindow windowToXWindow(long window) {
        synchronized(winMap) {
            return (XBaseWindow) winMap.get(new Long(window));
        }
    }

    static void addEventDispatcher(long window, XEventDispatcher dispatcher) {
        synchronized(winToDispatcher) {
            Long key = new Long(window);
            Collection dispatchers = (Collection)winToDispatcher.get(key);
            if (dispatchers == null) {
                dispatchers = new Vector();
                winToDispatcher.put(key, dispatchers);
            }
            dispatchers.add(dispatcher);
        }            
    }
    static void removeEventDispatcher(long window, XEventDispatcher dispatcher) {
        synchronized(winToDispatcher) {
            Long key = new Long(window);
            Collection dispatchers = (Collection)winToDispatcher.get(key);
            if (dispatchers != null) {
                dispatchers.remove(dispatcher);
            }
        }            
    }
    void dispatchEvent(XAnyEvent ev) {

        XBaseWindow.dispatchToWindow(ev);
        
        Collection dispatchers = null;
        synchronized(winToDispatcher) {
            Long key = new Long(ev.get_window());
            dispatchers = (Collection)winToDispatcher.get(key);
            if (dispatchers != null) { // Clone it to avoid synchronization during dispatching
                dispatchers = new Vector(dispatchers);
            }
        }
        if (dispatchers != null) {
            Iterator iter = dispatchers.iterator();
            while (iter.hasNext()) {
                XEventDispatcher disp = (XEventDispatcher)iter.next();
                disp.dispatchEvent(ev);
            }
        }
    }

    static void processException(Throwable thr) {
        System.err.println("Exception on Toolkit thread: " + thr);
        thr.printStackTrace();
    }

    static XFramePeer getParentFramePeer(Component comp) {
        if (comp == null) {
            return null;
        }
        ComponentPeer peer = comp.getPeer();
        synchronized(comp.getTreeLock()) {
            while (comp != null && !(peer instanceof XFramePeer)) {
                comp = comp.getParent();
                peer = comp.getPeer();
            }
        }
        if (peer != null && peer instanceof XFramePeer) {
            return (XFramePeer)peer;
        } else {
            return null;
        }
    }

    public void run() {
        initUIDefaults(); 
        run(PRIMARY_LOOP);
    }

    public void run(boolean loop)
    {
        XAnyEvent ev = new XAnyEvent(tempptr);
        while(true) {
            awtLock();
            try {        
                if (loop == SECONDARY_LOOP) {
                    // In the secondary loop we may have already aquired awt_lock 
                    // several times, so waitForEvents() might be unable to release
                    // the awt_lock and this causes lock up.
                    // For now, we just avoid waitForEvents in the secondary loop.
                    if (!XlibWrapper.XNextSecondaryLoopEvent(getDisplay(),ev.pData)) {
                        break;
                    }
                } else {
                    callTimeoutTasks();
                    // If no events are queued, waitForEvents() causes calls to
                    // awtUnlock(), awtJNI_ThreadYield, poll, awtLock(),
                    // so it spends most of its time in poll, without holding the lock.
                    while ((XlibWrapper.XEventsQueued(getDisplay(), XlibWrapper.QueuedAfterReading) == 0) &&
                           (XlibWrapper.XEventsQueued(getDisplay(), XlibWrapper.QueuedAfterFlush) == 0)) {
                        callTimeoutTasks();
                        waitForEvents();
                    }
                    XlibWrapper.XNextEvent(getDisplay(),ev.pData);
                }

                if (XDropTargetEventProcessor.processEvent(ev) ||
                    XDragSourceContextPeer.processEvent(ev)) {
                    continue;
                }

                if (eventLog.isLoggable(Level.FINER)) {
                    eventLog.finer(ev.toString());
                }

                // Check if input method consumes the event
                long w = 0;
                if (windowToXWindow(ev.get_window()) != null) {
                    Component owner = 
                        XKeyboardFocusManagerPeer.getCurrentNativeFocusOwner();
                    if (owner != null) {
                        XWindow ownerWindow = (XWindow)owner.getPeer();
                        if (ownerWindow != null) {
                            w = ownerWindow.getContentWindow();
                        }
		    }
                }
                if (XlibWrapper.XFilterEvent(ev.pData, w)) {
                    continue;
                }
                
                dispatchEvent(ev);

                XlibWrapper.XFlush(getDisplay());                
            } catch (ThreadDeath td) {
                XBaseWindow.ungrabInput();
                return;
            } catch (Throwable thr) {
                XBaseWindow.ungrabInput();
                processException(thr);
            } finally {
                awtUnlock();
            }
        }
    }

    static int getDefaultScreenWidth() {
        if (screenWidth == -1) {
            long display = getDisplay();
            synchronized(getAWTLock()) {
                screenWidth = (int) XlibWrapper.DisplayWidth(display, XlibWrapper.DefaultScreen(display));
            }
        }
        return screenWidth;
    }

    static int getDefaultScreenHeight() {
        if (screenHeight == -1) {
            long display = getDisplay();
            synchronized(getAWTLock()) {
                screenHeight = (int) XlibWrapper.DisplayHeight(display, XlibWrapper.DefaultScreen(display));
            }
        }
        return screenHeight;
    }
    
    protected int getScreenWidth() {
        return getDefaultScreenWidth();
    }

    protected int getScreenHeight() {
        return getDefaultScreenHeight();
    }

    // Need this for XMenuItemPeer.
    protected static final Object targetToPeer(Object target) {
        Object p=null;
        if (target != null && !GraphicsEnvironment.isHeadless()) {
            p = specialPeerMap.get(target);
        }
        if (p != null) return p;
        else   
            return SunToolkit.targetToPeer(target);
    }

    // Need this for XMenuItemPeer.
    protected static final void targetDisposedPeer(Object target, Object peer) {
        SunToolkit.targetDisposedPeer(target, peer);
    }

    public RobotPeer createRobot(Robot target, GraphicsDevice screen) {
        return new XRobotPeer(screen.getDefaultConfiguration());
    }


  /*
     * On X, support for dynamic layout on resizing is governed by the
     * window manager.  If the window manager supports it, it happens
     * automatically.  The setter method for this property is
     * irrelevant on X.
     */
    public void setDynamicLayout(boolean b) {
        dynamicLayoutSetting = b;
    }

    protected boolean isDynamicLayoutSet() {
        return dynamicLayoutSetting;
    }

    /* Called from isDynamicLayoutActive() and from 
     * lazilyLoadDynamicLayoutSupportedProperty()
     */
    protected boolean isDynamicLayoutSupported() {
        return XWM.getWM().supportsDynamicLayout();
    }

    public boolean isDynamicLayoutActive() {
        return isDynamicLayoutSupported();
    }


    public FontPeer getFontPeer(String name, int style){
        return new XFontPeer(name, style); 
    }

    public DragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) throws InvalidDnDOperationException {
        return XDragSourceContextPeer.createDragSourceContextPeer(dge);
    }

    public <T extends DragGestureRecognizer> T
	createDragGestureRecognizer(Class<T> recognizerClass,
				    DragSource ds,
				    Component c, 
				    int srcActions, 
				    DragGestureListener dgl)
    {
        if (MouseDragGestureRecognizer.class.equals(recognizerClass))
            return (T)new XMouseDragGestureRecognizer(ds, c, srcActions, dgl);
        else
            return null;
    }

    public CheckboxMenuItemPeer createCheckboxMenuItem(CheckboxMenuItem target) {
        XCheckboxMenuItemPeer peer = new XCheckboxMenuItemPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public MenuItemPeer createMenuItem(MenuItem target) {
        XMenuItemPeer peer = new XMenuItemPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public TextFieldPeer createTextField(TextField target) {
        TextFieldPeer  peer = new XTextFieldPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public LabelPeer createLabel(Label target) {
        LabelPeer  peer = new XLabelPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public ListPeer createList(List target) {
        ListPeer peer = new XListPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public CheckboxPeer createCheckbox(Checkbox target) {
        CheckboxPeer peer = new XCheckboxPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public ScrollbarPeer createScrollbar(Scrollbar target) {
        XScrollbarPeer peer = new XScrollbarPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public ScrollPanePeer createScrollPane(ScrollPane target) {
        XScrollPanePeer peer = new XScrollPanePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public TextAreaPeer createTextArea(TextArea target) {
        TextAreaPeer peer = new XTextAreaPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public ChoicePeer createChoice(Choice target) {
        XChoicePeer peer = new XChoicePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public CanvasPeer createCanvas(Canvas target) {
        XCanvasPeer peer = new XCanvasPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public PanelPeer createPanel(Panel target) {
        PanelPeer peer = new XPanelPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public WindowPeer createWindow(Window target) {
        WindowPeer peer = new XWindowPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public DialogPeer createDialog(Dialog target) {
        DialogPeer peer = new XDialogPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public FileDialogPeer createFileDialog(FileDialog target) {
        FileDialogPeer peer = new XFileDialogPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public MenuBarPeer createMenuBar(MenuBar target) {
        XMenuBarPeer peer = new XMenuBarPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public MenuPeer createMenu(Menu target) {
        XMenuPeer peer = new XMenuPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public PopupMenuPeer createPopupMenu(PopupMenu target) {
        XPopupMenuPeer peer = new XPopupMenuPeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    public synchronized MouseInfoPeer getMouseInfoPeer() {
        if (xPeer == null) {
            xPeer = new XMouseInfoPeer();
        }
        return xPeer;
    }

    public XEmbeddedFramePeer createEmbeddedFrame(XEmbeddedFrame target)
    {
        XEmbeddedFramePeer peer = new XEmbeddedFramePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }

    XEmbedChildProxyPeer createEmbedProxy(XEmbedChildProxy target) {
        XEmbedChildProxyPeer peer = new XEmbedChildProxyPeer(target);
        targetCreatedPeer(target, peer);
        return peer;        
    }
    
    public KeyboardFocusManagerPeer createKeyboardFocusManagerPeer(KeyboardFocusManager manager) throws HeadlessException {
        XKeyboardFocusManagerPeer peer = new XKeyboardFocusManagerPeer(manager);
        return peer;
    }

    /**
     * Returns a new custom cursor.
     */
    public Cursor createCustomCursor(Image cursor, Point hotSpot, String name)
      throws IndexOutOfBoundsException {
        return new XCustomCursor(cursor, hotSpot, name);
    }

    /**
     * Returns the supported cursor size
     */
    public Dimension getBestCursorSize(int preferredWidth, int preferredHeight) {
        return XCustomCursor.getBestCursorSize(
                                               java.lang.Math.max(1,preferredWidth), java.lang.Math.max(1,preferredHeight));
    }



    public Map mapInputMethodHighlight(InputMethodHighlight highlight)     {
        return XInputMethod.mapInputMethodHighlight(highlight);
    }

    public  Clipboard getSystemClipboard() {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkSystemClipboardAccess();
        }
        synchronized (this) {
            if (clipboard == null) {
                clipboard = new XClipboard("System", "CLIPBOARD");
            }
        }
        return clipboard;
    }

    public Clipboard getSystemSelection() {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkSystemClipboardAccess();
        }
        synchronized (this) {
            if (selection == null) {
                selection = new XClipboard("Selection", "PRIMARY");
            }
        }
        return selection;
    }

    public void beep() {
        synchronized (getAWTLock()) {
            XlibWrapper.XBell(getDisplay(), 0);
        }
    }

    static String getSystemProperty(final String name) {
        return (String)AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return System.getProperty(name);
                }
            });
    }

    public PrintJob getPrintJob(final Frame frame, final String doctitle,
                                final Properties props) {

        if (GraphicsEnvironment.isHeadless()) {
            throw new IllegalArgumentException();
        }

        PrintJob2D printJob = new PrintJob2D(frame, doctitle, props);

        if (printJob.printDialog() == false) {
            printJob = null;
        }
        return printJob;   
    }
    
    public PrintJob getPrintJob(final Frame frame, final String doctitle,
				final JobAttributes jobAttributes,
				final PageAttributes pageAttributes) {


        if (GraphicsEnvironment.isHeadless()) {
            throw new IllegalArgumentException();
        }

        PrintJob2D printJob = new PrintJob2D(frame, doctitle,
                                             jobAttributes, pageAttributes);

        if (printJob.printDialog() == false) {
            printJob = null;
        }

        return printJob;
    }

    static void XSync() {
        synchronized (getAWTLock()) {
            XlibWrapper.XSync(getDisplay(),0);
        }        
    }

    public int getScreenResolution() {
        long display = getDisplay();
        synchronized (getAWTLock()) {
            return (int) ((XlibWrapper.DisplayWidth(display, 
                XlibWrapper.DefaultScreen(display)) * 25.4) /
                    XlibWrapper.DisplayWidthMM(display, 
                XlibWrapper.DefaultScreen(display)));
        }
    }

    static native long getDefaultXColormap();
    static native long getDefaultScreenData();

    static native ColorModel makeColorModel();
    static ColorModel screenmodel;

    static ColorModel getStaticColorModel() {
        if (screenmodel == null) {
            screenmodel = config.getColorModel ();
        }
        return screenmodel;
    }

    public ColorModel getColorModel() {
        return getStaticColorModel();
    }

    /**
     * Returns a new input method adapter descriptor for native input methods.
     */
    public InputMethodDescriptor getInputMethodAdapterDescriptor() throws AWTException {
        return new XInputMethodDescriptor();
    }

    static int getMultiClickTime() {
        if (awt_multiclick_time == 0) {
            initializeMultiClickTime();
        }
        return awt_multiclick_time;
    }
    static void initializeMultiClickTime() {
        synchronized (getAWTLock()) {
            try {
                String multiclick_time_query = XlibWrapper.XGetDefault(XToolkit.getDisplay(), "*", "multiClickTime");
                if (multiclick_time_query != null) {
                    awt_multiclick_time = (int)Long.parseLong(multiclick_time_query);
    //             awt_multiclick_time = XtGetMultiClickTime(awt_display);
                } else {
                    multiclick_time_query = XlibWrapper.XGetDefault(XToolkit.getDisplay(),
                                                                    "OpenWindows", "MultiClickTimeout");
                    if (multiclick_time_query != null) {
                        /* Note: OpenWindows.MultiClickTimeout is in tenths of 
                           a second, so we need to multiply by 100 to convert to
                           milliseconds */
                        awt_multiclick_time = (int)Long.parseLong(multiclick_time_query) * 100;
                    } else {
                        awt_multiclick_time = 200;
    //                 awt_multiclick_time = XtGetMultiClickTime(awt_display);
                    }
                }        
            } catch (NumberFormatException nf) {
                awt_multiclick_time = 200;            
            } catch (NullPointerException npe) {            
                awt_multiclick_time = 200;            
            }
        }
        if (awt_multiclick_time == 0) {
            awt_multiclick_time = 200;
        }
    }

    public boolean isFrameStateSupported(int state) 
      throws HeadlessException
    {
        if (state == Frame.NORMAL || state == Frame.ICONIFIED) {
            return true;
        } else {
            return XWM.getWM().supportsExtendedState(state);
        }
    }    
    
    static void dumpPeers() {
        if (log.isLoggable(Level.FINE)) {
            System.err.println("Mapped windows:");
            Iterator iter = winMap.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                System.err.println(entry.getKey() + "->" + entry.getValue());
                if (entry.getValue() instanceof XComponentPeer) {
                    Component target = (Component)((XComponentPeer)entry.getValue()).getTarget();
                    System.err.println("\ttarget: " + target);
                }
            }

            System.err.println("Mapped peers:");
            iter = SunToolkit.peerMap.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                System.err.println(entry.getKey() + "->" + entry.getValue());
            }
        
            System.err.println("Mapped special peers:");
            iter = specialPeerMap.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                System.err.println(entry.getKey() + "->" + entry.getValue());
            }    

            System.err.println("Mapped dispatchers:");
            iter = winToDispatcher.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                System.err.println(entry.getKey() + "->" + entry.getValue());
            }    
        }
    }

    /* Protected with awt_lock. */
    private static boolean initialized;
    private static boolean timeStampUpdated;
    private static long timeStamp;

    private static final XEventDispatcher timeFetcher = 
    new XEventDispatcher() {
            public void dispatchEvent(IXAnyEvent ev) {
                long ptr = ev.getPData();
                switch (ev.get_type()) {
                  case PropertyNotify: 
                      XPropertyEvent xpe = new XPropertyEvent(ptr);

                      synchronized (getAWTLock()) {
                          timeStamp = xpe.get_time();
                          timeStampUpdated = true;
                          getAWTLock().notifyAll();
                      }

                      break;
                }
            }
        };

    private static XAtom _XA_JAVA_TIME_PROPERTY_ATOM;

    static long getCurrentServerTime() {
        synchronized (getAWTLock()) {
            try {
                if (!initialized) {
                    XToolkit.addEventDispatcher(XWindow.getXAWTRootWindow().getWindow(), 
                                                timeFetcher);
                    _XA_JAVA_TIME_PROPERTY_ATOM = XAtom.get("_SUNW_JAVA_AWT_TIME");
                    initialized = true;
                }
                timeStampUpdated = false;
                XlibWrapper.XChangeProperty(XToolkit.getDisplay(), 
                                            XWindow.getXAWTRootWindow().getWindow(),
                                            _XA_JAVA_TIME_PROPERTY_ATOM.getAtom(), XAtom.XA_ATOM, 32, 
                                            PropModeAppend,
                                            0, 0);
                XlibWrapper.XFlush(XToolkit.getDisplay());
                
                if (isToolkitThread()) {
                    XEvent event = new XEvent();
                    try {
                        XlibWrapper.XWindowEvent(XToolkit.getDisplay(),
                                                 XWindow.getXAWTRootWindow().getWindow(),
                                                 XConstants.PropertyChangeMask,
                                                 event.pData);
                        XPropertyEvent pe = new XPropertyEvent(event.pData);
                        timeFetcher.dispatchEvent(pe);
                    }
                    finally {
                        if (event != null) {
                            event.dispose();
                        }
                    }
                }
                else {
                    while (!timeStampUpdated) {
                        getAWTLock().wait();
                    }
                }
            } catch (InterruptedException ie) {
            // Note: the returned timeStamp can be incorrect in this case.
                if (log.isLoggable(Level.FINE)) log.fine("Catched exception, timeStamp may not be correct (ie = " + ie + ")");
            }
        }
        return timeStamp;
    }
    protected void initializeDesktopProperties() {
        desktopProperties.put("DnD.Autoscroll.initialDelay",     new Integer(50));
        desktopProperties.put("DnD.Autoscroll.interval",         new Integer(50));
        desktopProperties.put("DnD.Autoscroll.cursorHysteresis", new Integer(5));
        // Don't want to call getMultiClickTime() if we are headless
        if (!GraphicsEnvironment.isHeadless()) {
            desktopProperties.put("awt.multiClickInterval",
                                  new Integer(getMultiClickTime()));
            desktopProperties.put("awt.mouse.numButtons",
                                  new Integer(getNumMouseButtons()));
        }
    }

    private int getNumMouseButtons() {
        synchronized (getAWTLock()) {
            return XlibWrapper.XGetPointerMapping(XToolkit.getDisplay(), 0, 0);
        }
    }

    private final static String prefix  = "DnD.Cursor.";
    private final static String postfix = ".32x32";
    private static final String dndPrefix  = "DnD.";

    protected Object lazilyLoadDesktopProperty(String name) {
        if (name.startsWith(prefix)) {
            String cursorName = name.substring(prefix.length(), name.length()) + postfix;

            try {
                return Cursor.getSystemCustomCursor(cursorName);
            } catch (AWTException awte) {
                System.err.println("cannot load system cursor: " + cursorName);

                return null;
            }
        }

        if (name.equals("awt.dynamicLayoutSupported")) {
            return  new Boolean(isDynamicLayoutSupported());
        }

        if (!loadedXSettings && (name.startsWith("gnome.") || name.startsWith(dndPrefix))) {
            loadedXSettings = true;
            if (!GraphicsEnvironment.isHeadless()) {
                loadXSettings();
                return desktopProperties.get(name);
            }
        }

        return super.lazilyLoadDesktopProperty(name);
    }
    
    
    void loadXSettings() {
       xs = new XAWTXSettings();
    }

      /**
     * Callback from the native side indicating some, or all, of the
     * desktop properties have changed and need to be reloaded.
     * <code>data</code> is the byte array directly from the x server and
     * may be in little endian format.
     * <p>
     * NB: This could be called from any thread if triggered by
     * <code>loadXSettings</code>.  It is called from the toolkit
     * thread if triggered by an XSETTINGS change.
     */
    void parseXSettings(int screen_XXX_ignored,Map updatedSettings) {

        if (updatedSettings == null || updatedSettings.isEmpty()) {
            return;
        }

        Iterator i = updatedSettings.entrySet().iterator();
        while (i.hasNext()) {
            Map.Entry e = (Map.Entry)i.next();
            String name = (String)e.getKey();

            name = "gnome." + name;
            setDesktopProperty(name, e.getValue());
            log.fine("name = " + name + " value = " + e.getValue());

            // XXX: we probably want to do something smarter.  In
            // particular, "Net" properties are of interest to the
            // "core" AWT itself.  E.g.
            // 
            // Net/DndDragThreshold -> ???
            // Net/DoubleClickTime  -> awt.multiClickInterval
        }

        Integer dragThreshold = null;
        synchronized (this) {
            dragThreshold = (Integer)desktopProperties.get("gnome.Net/DndDragThreshold");
        }
        if (dragThreshold != null) {
            setDesktopProperty("DnD.gestureMotionThreshold", dragThreshold);
        }

    }



    static int altMask;
    static int metaMask;
    static int numLockMask;
    static int modeSwitchMask;    

    /* Like XKeysymToKeycode, but ensures that keysym is the primary
    * symbol on the keycode returned.  Returns zero otherwise.
    */
    static int keysymToPrimaryKeycode(long sym) {
        synchronized (getAWTLock()) {
            int code = XlibWrapper.XKeysymToKeycode(getDisplay(), sym);
            if (code == 0) {
                return 0;
            }
            long primary = XlibWrapper.XKeycodeToKeysym(getDisplay(), code, 0);
            if (sym != primary) {
                return 0;
            }
            return code;
        }
    }

    /* Assign meaning - alt, meta, etc. - to X modifiers mod1 ... mod5.  
     * Only consider primary symbols on keycodes attached to modifiers.
     */
    static void setupModifierMap() {
        final int metaL = keysymToPrimaryKeycode(XKeySymConstants.XK_Meta_L);
        final int metaR = keysymToPrimaryKeycode(XKeySymConstants.XK_Meta_R);
        final int altL = keysymToPrimaryKeycode(XKeySymConstants.XK_Alt_L);
        final int altR = keysymToPrimaryKeycode(XKeySymConstants.XK_Alt_R);
        final int numLock = keysymToPrimaryKeycode(XKeySymConstants.XK_Num_Lock);
        final int modeSwitch = keysymToPrimaryKeycode(XKeySymConstants.XK_Mode_switch);

        final int modmask[] = { ShiftMask, LockMask, ControlMask, Mod1Mask,
            Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };  
            
        log.fine("In setupModifierMap");
        synchronized (getAWTLock()) {
            XModifierKeymap modmap = new XModifierKeymap(
                 XlibWrapper.XGetModifierMapping(getDisplay()));

            int nkeys = modmap.get_max_keypermod();

            long map_ptr = modmap.get_modifiermap();

            for (int modn = XConstants.Mod1MapIndex; 
                 modn <= XConstants.Mod5MapIndex;
                 ++modn)
            { 
                for (int i = 0; i < nkeys; ++i) {
                    /* for each keycode attached to this modifier */
                    int keycode = Native.getUByte(map_ptr, modn * nkeys + i);

                    if (keycode == 0) {
                        break;
                    }
                    if (metaMask == 0 && 
                        (keycode == metaL || keycode == metaR))
                    {
                        metaMask = modmask[modn];
                        break;
                    }
                    if (altMask == 0 && (keycode == altL || keycode == altR)) {
                        altMask = modmask[modn];
                        break;
                    }
                    if (numLockMask == 0 && keycode == numLock) {
                        numLockMask = modmask[modn];
                        break;
                    }
                    if (modeSwitchMask == 0 && keycode == modeSwitch) {
                        modeSwitchMask = modmask[modn];
                        break;
                    }
                    continue;
                }
            }
            XlibWrapper.XFreeModifiermap(modmap.pData);
        }
        if (log.isLoggable(Level.FINE)) {
            log.fine("metaMask = " + metaMask);
            log.fine("altMask = " + altMask);
            log.fine("numLockMask = " + numLockMask);
            log.fine("modeSwitchMask = " + modeSwitchMask);
        }
    }


    private static SortedMap timeoutTasks;

    /**
     * Removed the task from the list of waiting-to-be called tasks.
     * If the task has been scheduled several times removes only first one.
     */
    static void remove(Runnable task) {
        if (task == null) {
            throw new NullPointerException("task is null");
        }
        synchronized (getAWTLock()) {
            if (timeoutTaskLog.isLoggable(Level.FINER)) {
                timeoutTaskLog.finer("Removing task " + task);
            }
            if (timeoutTasks == null) {
                if (timeoutTaskLog.isLoggable(Level.FINER)) {
                    timeoutTaskLog.finer("Task is not scheduled");
                }
                return;
            }
            Collection values = timeoutTasks.values();
            Iterator iter = values.iterator();
            while (iter.hasNext()) {
                java.util.List list = (java.util.List)iter.next();
                boolean removed = false;
                if (list.contains(task)) {
                    list.remove(task);
                    if (list.isEmpty()) {
                        iter.remove();
                    }
                    break;
                }
            }
        }
    }

    /**
     * Registers a Runnable which <code>run()</code> method will be called
     * once on the toolkit thread when a specified interval of time elapses.
     *
     * @param task a Runnable which <code>run</code> method will be called
     *        on the toolkit thread when <code>interval</code> milliseconds
     *        elapse
     * @param interval an interal in milliseconds
     *
     * @throw NullPointerException if <code>task</code> is <code>null</code>
     * @throws IllegalArgumentException if <code>interval</code> is not positive
     */  
    static void schedule(Runnable task, long interval) {
        if (task == null) {
            throw new NullPointerException("task is null");
        }
        if (interval <= 0) {
            throw new IllegalArgumentException("interval " + interval + " is not positive");
        }
        
        synchronized (getAWTLock()) {
            if (timeoutTaskLog.isLoggable(Level.FINER)) {
                timeoutTaskLog.finer("XToolkit.schedule(): current time=" +
                        System.currentTimeMillis() + ";  interval=" + interval +
                        ";  task being added=" + task + ";  tasks before addition=" +
                        timeoutTasks);
            }

            if (timeoutTasks == null) {
                timeoutTasks = new TreeMap();
            }

            Long time = new Long(System.currentTimeMillis() + interval);
            java.util.List tasks = (java.util.List)timeoutTasks.get(time);
            if (tasks == null) {
                tasks = new ArrayList(1);
                timeoutTasks.put(time, tasks);
            }
            tasks.add(task);
        } 
    }

    /**
     * Executes mature timeout tasks registered with schedule().
     * Called from run() under awtLock.
     */
    private static void callTimeoutTasks() {
        if (timeoutTaskLog.isLoggable(Level.FINER)) {
            timeoutTaskLog.finer("XToolkit.callTimeoutTasks(): current time=" +
                    System.currentTimeMillis() + ";  tasks=" + timeoutTasks);
        }

        if (timeoutTasks == null || timeoutTasks.isEmpty()) {
            return;
        }

        Long currentTime = new Long(System.currentTimeMillis());
        Long time = (Long)timeoutTasks.firstKey();

        while (time.compareTo(currentTime) <= 0) {
            java.util.List tasks = (java.util.List)timeoutTasks.remove(time);

            for (Iterator iter = tasks.iterator(); iter.hasNext();) {
                Runnable task = (Runnable)iter.next();

                if (timeoutTaskLog.isLoggable(Level.FINER)) {
                    timeoutTaskLog.finer("XToolkit.callTimeoutTasks(): current time=" +
                                         currentTime + ";  about to run task=" + task);
                }

                try {
                    task.run();
                } catch (ThreadDeath td) {
                    throw td;
                } catch (Throwable thr) {
                    processException(thr);
                }
            }

            if (timeoutTasks.isEmpty()) {
                break;
            }
            time = (Long)timeoutTasks.firstKey();
        }
    }

    static long getAwtDefaultFg() {
        return awt_defaultFg;
    }

    static boolean isLeftMouseButton(MouseEvent me) {
        switch (me.getID()) {
          case MouseEvent.MOUSE_PRESSED: 
          case MouseEvent.MOUSE_RELEASED:
              return (me.getButton() == MouseEvent.BUTTON1);
          case MouseEvent.MOUSE_ENTERED:
          case MouseEvent.MOUSE_EXITED:
          case MouseEvent.MOUSE_CLICKED:
              return ((me.getModifiersEx() & InputEvent.BUTTON1_DOWN_MASK) != 0);
        }
        return false;
    }

    /**
     * Gets field <code>fieldName</code> from class <code>clz</code> and
     * makes it accessible.
     */
    static Field getField(final Class clz, final String fieldName) {
        Field res = null;
        try {
            res = (Field)AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws Exception {
                        Field f = clz.getDeclaredField(fieldName);
                        f.setAccessible(true);
                        return f;
                    }
                });       
        } catch (PrivilegedActionException ex) {
            ex.printStackTrace();
        }
        return res;
    }

    static Method getMethod(final Class clz, final String methodName, final Class[] params) {
        Method res = null;
        try {
            res = (Method)AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws Exception {
                        Method m = clz.getDeclaredMethod(methodName, params);
                        m.setAccessible(true);
                        return m;
                    }
                });       
        } catch (PrivilegedActionException ex) {
            ex.printStackTrace();
        }
        return res;        
    }

    static long reset_time_utc;
    static final long WRAP_TIME_MILLIS = Integer.MAX_VALUE;

    /*
     * This function converts between the X server time (number of milliseconds
     * since the last server reset) and the UTC time for the 'when' field of an
     * InputEvent (or another event type with a timestamp).
     */
    static long nowMillisUTC_offset(long server_offset) {
        // ported from awt_util.c
        /*
         * Because Time is of type 'unsigned long', it is possible that Time will
         * never wrap when using 64-bit Xlib. However, if a 64-bit client
         * connects to a 32-bit server, I suspect the values will still wrap. So
         * we should not attempt to remove the wrap checking even if _LP64 is
         * true.
         */
        
        long current_time_utc = System.currentTimeMillis();
        if (log.isLoggable(Level.FINER)) {
            log.finer("reset_time=" + reset_time_utc + ", current_time=" + current_time_utc
                      + ", server_offset=" + server_offset + ", wrap_time=" + WRAP_TIME_MILLIS);
        }

        if ((current_time_utc - reset_time_utc) > WRAP_TIME_MILLIS) {
            reset_time_utc = System.currentTimeMillis() - getCurrentServerTime();
        }

        if (log.isLoggable(Level.FINER)) {
            log.finer("result = " + (reset_time_utc + server_offset));
        }
        return reset_time_utc + server_offset;
    }

    /**
     * @see sun.awt.SunToolkit#needsXEmbedImpl
     */
    protected boolean needsXEmbedImpl() {
        // XToolkit implements supports for XEmbed-client protocol and
        // requires the supports from the embedding host for it to work.
        return true;
    }

    static EventQueue getEventQueue(Object target) {
        AppContext appContext = targetToAppContext(target);
        if (appContext != null) {
            return (EventQueue)appContext.get(AppContext.EVENT_QUEUE_KEY);
        }
        return null;
    }

    static void removeSourceEvents(EventQueue queue, Object source, boolean removeAllEvents) {
        try {
            m_removeSourceEvents.invoke(queue, source, removeAllEvents);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

}
