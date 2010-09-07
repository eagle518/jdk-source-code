/*
 * @(#)MToolkit.java	1.187 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.*;
import java.awt.im.InputMethodHighlight;
import java.awt.im.spi.InputMethodDescriptor;
import java.awt.image.*;
import java.awt.peer.*;
import java.awt.datatransfer.Clipboard;
import java.awt.event.*;
import java.lang.reflect.*;
import java.lang.Math;
import java.io.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Properties;
import java.util.Map;
import java.util.Iterator;

import sun.awt.AppContext;
import sun.awt.AWTAutoShutdown;
import sun.awt.SunToolkit;
import sun.awt.UNIXToolkit;
import sun.awt.GlobalCursorManager;
import sun.awt.DebugHelper;
import sun.awt.datatransfer.DataTransferer;
 
import java.awt.dnd.DragSource;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.MouseDragGestureRecognizer;
import java.awt.dnd.InvalidDnDOperationException;
import java.awt.dnd.peer.DragSourceContextPeer;

import sun.awt.motif.MInputMethod;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11GraphicsEnvironment;
import sun.awt.XSettings;

import sun.awt.motif.MDragSourceContextPeer;

import sun.print.PrintJob2D;

import sun.misc.PerformanceLogger;

import sun.security.action.GetBooleanAction;

public class MToolkit extends UNIXToolkit implements Runnable {
    private static final DebugHelper dbg = DebugHelper.create(MToolkit.class);
    
    // the system clipboard - CLIPBOARD selection
    X11Clipboard clipboard;
    // the system selection - PRIMARY selection
    X11Clipboard selection;

    // Dynamic Layout Resize client code setting
    protected static boolean dynamicLayoutSetting = false;

    /**
     * True when the x settings have been loaded.
     */
    private boolean loadedXSettings;

    /**
     * XSETTINGS for the default screen.
     * <p>
     * <strong>XXX:</strong> see <code>MToolkit.parseXSettings</code>
     * and <code>awt_xsettings_update</code> in
     * <samp>awt_MToolkit.c</samp>
     */
    private XSettings xs;

    /*
     * Note: The MToolkit object depends on the static initializer
     * of X11GraphicsEnvironment to initialize the connection to
     * the X11 server.
     */
    static final X11GraphicsConfig config;

    private static final boolean motifdnd;

    static {
        if (GraphicsEnvironment.isHeadless()) {
            config = null;
        } else {
            config = (X11GraphicsConfig) (GraphicsEnvironment.
			     getLocalGraphicsEnvironment().
			     getDefaultScreenDevice().
			     getDefaultConfiguration());
	}

	/* Add font properties font directories to the X11 font path.
	 * Its called here *after* the X connection has been initialised
	 * and when we know that MToolkit is the one that will be used,
	 * since XToolkit doesn't need the X11 font path set
	 */
	X11GraphicsEnvironment.setNativeFontPath();

        motifdnd = ((Boolean)java.security.AccessController.doPrivileged(
            new GetBooleanAction("awt.dnd.motifdnd"))).booleanValue();
    }

    public static final String DATA_TRANSFERER_CLASS_NAME = "sun.awt.motif.MDataTransferer";

    public MToolkit() {
        super();
	if (PerformanceLogger.loggingEnabled()) {
	    PerformanceLogger.setTime("MToolkit construction");
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

            init(mainClassName);
            SunToolkit.setDataTransfererClassName(DATA_TRANSFERER_CLASS_NAME);

            Thread toolkitThread = new Thread(this, "AWT-Motif");
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
	
	    Runtime.getRuntime().addShutdownHook(
	    	new Thread(mainTG, new Runnable() {
	            public void run() {
		        shutdown();
		    }
		}, "Shutdown-Thread")
	    );

            /*
             * Fix for 4701990.
             * AWTAutoShutdown state must be changed before the toolkit thread
             * starts to avoid race condition.
             */
            AWTAutoShutdown.notifyToolkitThreadBusy();

            toolkitThread.start();
        }
    }

    public native void init(String mainClassName);
    public native void run();
    private native void shutdown();

    /*
     * Create peer objects.
     */

    public ButtonPeer createButton(Button target) {
	ButtonPeer peer = new MButtonPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public TextFieldPeer createTextField(TextField target) {
	TextFieldPeer peer = new MTextFieldPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public LabelPeer createLabel(Label target) {
	LabelPeer peer = new MLabelPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ListPeer createList(List target) {
	ListPeer peer = new MListPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CheckboxPeer createCheckbox(Checkbox target) {
	CheckboxPeer peer = new MCheckboxPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ScrollbarPeer createScrollbar(Scrollbar target) {
	ScrollbarPeer peer = new MScrollbarPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ScrollPanePeer createScrollPane(ScrollPane target) {
	ScrollPanePeer peer = new MScrollPanePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public TextAreaPeer createTextArea(TextArea target) {
	TextAreaPeer peer = new MTextAreaPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public ChoicePeer createChoice(Choice target) {
	ChoicePeer peer = new MChoicePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public FramePeer  createFrame(Frame target) {
	FramePeer peer = new MFramePeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CanvasPeer createCanvas(Canvas target) {
	CanvasPeer peer = new MCanvasPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public PanelPeer createPanel(Panel target) {
	PanelPeer peer = new MPanelPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public WindowPeer createWindow(Window target) {
	WindowPeer peer = new MWindowPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public DialogPeer createDialog(Dialog target) {
	DialogPeer peer = new MDialogPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public FileDialogPeer createFileDialog(FileDialog target) {
	FileDialogPeer peer = new MFileDialogPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuBarPeer createMenuBar(MenuBar target) {
	MenuBarPeer peer = new MMenuBarPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuPeer createMenu(Menu target) {
	MenuPeer peer = new MMenuPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public PopupMenuPeer createPopupMenu(PopupMenu target) {
	PopupMenuPeer peer = new MPopupMenuPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MenuItemPeer createMenuItem(MenuItem target) {
	MenuItemPeer peer = new MMenuItemPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public CheckboxMenuItemPeer createCheckboxMenuItem(CheckboxMenuItem target) {
	CheckboxMenuItemPeer peer = new MCheckboxMenuItemPeer(target);
	targetCreatedPeer(target, peer);
	return peer;
    }

    public MEmbeddedFramePeer createEmbeddedFrame(MEmbeddedFrame target)
    {
        MEmbeddedFramePeer peer = new MEmbeddedFramePeer(target);
        targetCreatedPeer(target, peer);
        return peer;
    }


    public FontPeer getFontPeer(String name, int style){
	return new MFontPeer(name, style);
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
    protected native boolean isDynamicLayoutSupportedNative();

    public boolean isDynamicLayoutActive() {
        return isDynamicLayoutSupportedNative();
    }

    public native boolean isFrameStateSupported(int state);

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

    public native int getScreenResolution();

    public Insets getScreenInsets(GraphicsConfiguration gc) {
        return new Insets(0,0,0,0);
    }

    protected native int getScreenWidth();
    protected native int getScreenHeight();

    public FontMetrics getFontMetrics(Font font) {
	/*
	// REMIND: platform font flag should be obsolete soon
        if (!RasterOutputManager.usesPlatformFont()) {
            return super.getFontMetrics(font);
        } else {
            return X11FontMetrics.getFontMetrics(font);
        }
	*/
	return super.getFontMetrics(font);
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

    public native void beep();

    public  Clipboard getSystemClipboard() {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
	  security.checkSystemClipboardAccess();
	}
        synchronized (this) {
            if (clipboard == null) {
                clipboard = new X11Clipboard("System", "CLIPBOARD");
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
                selection = new X11Clipboard("Selection", "PRIMARY");
            }
        }
        return selection;
    }
    
    public boolean getLockingKeyState(int key) {
        if (! (key == KeyEvent.VK_CAPS_LOCK || key == KeyEvent.VK_NUM_LOCK ||
               key == KeyEvent.VK_SCROLL_LOCK || key == KeyEvent.VK_KANA_LOCK)) {
            throw new IllegalArgumentException("invalid key for Toolkit.getLockingKeyState");
        }
        return getLockingKeyStateNative(key);
    }
    
    public native boolean getLockingKeyStateNative(int key);

    public native void loadSystemColors(int[] systemColors);

    /**
     * Give native peers the ability to query the native container 
     * given a native component (e.g. the direct parent may be lightweight).
     */
    public static Container getNativeContainer(Component c) {
	return Toolkit.getNativeContainer(c);
    }

    protected static final Object targetToPeer(Object target) {
        return SunToolkit.targetToPeer(target);
    }

    protected static final void targetDisposedPeer(Object target, Object peer) {
        SunToolkit.targetDisposedPeer(target, peer);
    }

    public DragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) throws InvalidDnDOperationException {
        if (MToolkit.useMotifDnD()) {
            return MDragSourceContextPeer.createDragSourceContextPeer(dge);
        } else {
            return X11DragSourceContextPeer.createDragSourceContextPeer(dge);
        }
    }

    public <T extends DragGestureRecognizer> T
	createDragGestureRecognizer(Class<T> abstractRecognizerClass,
				    DragSource ds, Component c, int srcActions,
				    DragGestureListener dgl)
    {
	if (MouseDragGestureRecognizer.class.equals(abstractRecognizerClass))
	    return (T)new MMouseDragGestureRecognizer(ds, c, srcActions, dgl);
	else
            return null;
    }

    /**
     * Returns a new input method adapter descriptor for native input methods.
     */
    public InputMethodDescriptor getInputMethodAdapterDescriptor() throws AWTException {
	return new MInputMethodDescriptor();
    }

    /**
     * Returns a style map for the input method highlight.
     */
    public Map mapInputMethodHighlight(InputMethodHighlight highlight) {
	return MInputMethod.mapInputMethodHighlight(highlight);
    }

    /**
     * Returns a new custom cursor.
     */
    public Cursor createCustomCursor(Image cursor, Point hotSpot, String name)
        throws IndexOutOfBoundsException {
        return new MCustomCursor(cursor, hotSpot, name);
    }

    /**
     * Returns the supported cursor size
     */
    public Dimension getBestCursorSize(int preferredWidth, int preferredHeight) {
        return MCustomCursor.getBestCursorSize(
            java.lang.Math.max(1,preferredWidth), java.lang.Math.max(1,preferredHeight));
    }

    public int getMaximumCursorColors() {
        return 2;  // Black and white.
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
            return lazilyLoadDynamicLayoutSupportedProperty(name);
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

    /*
     * Called from lazilyLoadDesktopProperty because we may not know if 
     * the user has quit the previous window manager and started another. 
     */
    protected Boolean lazilyLoadDynamicLayoutSupportedProperty(String name) {
        boolean nativeDynamic = isDynamicLayoutSupportedNative();

        if (dbg.on) {
            dbg.print("In MTK.lazilyLoadDynamicLayoutSupportedProperty()" +
              "   nativeDynamic == " + nativeDynamic); 
        }

        return new Boolean(nativeDynamic);
    }

    private native int getMulticlickTime();
    private native int getNumMouseButtons();

    protected void initializeDesktopProperties() {
	desktopProperties.put("DnD.Autoscroll.initialDelay",     new Integer(50));
	desktopProperties.put("DnD.Autoscroll.interval",         new Integer(50));
	desktopProperties.put("DnD.Autoscroll.cursorHysteresis", new Integer(5));

        /* As of 1.4, no wheel mice are supported on Solaris
         * however, they are on Linux, and there isn't a way to detect them,
         * so we leave this property unset to indicate we're not sure if there's
         * a wheel mouse or not.
         */
	//desktopProperties.put("awt.wheelMousePresent", new Boolean(false));

        // We don't want to call getMultilclickTime() if we're headless
        if (!GraphicsEnvironment.isHeadless()) {
            desktopProperties.put("awt.multiClickInterval",
                                  new Integer(getMulticlickTime()));
            desktopProperties.put("awt.mouse.numButtons",
                                  new Integer(getNumMouseButtons()));
        }
    }

    public RobotPeer createRobot(Robot target, GraphicsDevice screen) {
	/* 'target' is unused for now... */
	return new MRobotPeer(screen.getDefaultConfiguration());
    }

    static boolean useMotifDnD() {
        return motifdnd;
    }

    //
    // The following support Gnome's equivalent of desktop properties.
    // A writeup of this can be found at:
    // http://www.freedesktop.org/standards/xsettings/xsettings.html
    //

    /**
     * Triggers a callback to parseXSettings with the x settings values
     * from the window server. Note that this will NOT call
     * parseXSettings if we are not running on a GNOME desktop.
     */
    private native void loadXSettings();

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
    private void parseXSettings(int screen_XXX_ignored, byte[] data) {
	// XXX: notyet: map screen -> per screen XSettings object
	// for now native code only calls us for default screen
	// see awt_MToolkit.c awt_xsettings_update().
	if (xs == null) {
	    xs = new XSettings();
	}

	Map updatedSettings = xs.update(data);
	if (updatedSettings == null || updatedSettings.isEmpty()) {
	    return;
	}

	Iterator i = updatedSettings.entrySet().iterator();
	while (i.hasNext()) {
	    Map.Entry e = (Map.Entry)i.next();
	    String name = (String)e.getKey();

	    name = "gnome." + name;
	    setDesktopProperty(name, e.getValue());

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

    protected boolean needsXEmbedImpl() {
        return true;
    }
} // class MToolkit

