/*
 * @(#)SunToolkit.java	1.89 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.*;
import java.awt.dnd.*;
import java.awt.dnd.peer.DragSourceContextPeer;
import java.awt.peer.*;
import java.awt.event.WindowEvent;
import java.awt.im.spi.InputMethodDescriptor;
import java.awt.image.*;
import java.awt.geom.AffineTransform;
import java.io.*;
import java.net.URL;
import java.net.JarURLConnection;
import java.util.*;
import java.util.logging.*;
import sun.misc.SoftCache;
import sun.font.FontDesignMetrics;
import sun.awt.im.InputContext;
import sun.awt.im.SimpleInputMethodWindow;
import sun.awt.image.*;
import sun.security.action.GetPropertyAction;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;

public abstract class SunToolkit extends Toolkit
    implements WindowClosingSupport, WindowClosingListener,
    ComponentFactory, InputMethodSupport {

    private static final Logger log = Logger.getLogger("sun.awt.SunToolkit");

    /* Force the debug helper classes to initialize */
    {
	DebugHelper.init();
    }

    private static boolean hotjavaUrlCache = false; // REMIND: UGH!!!!!
    private static Field syncLWRequestsField;
    private static Method  wakeupMethod;
    private static Field componentKeyField;
    private static Field menuComponentKeyField;
    /* The key to put()/get() the PostEventQueue into/from the AppContext.
     */
    private static final String POST_EVENT_QUEUE_KEY = "PostEventQueue";
    
    public SunToolkit() {
	/* If awt.threadgroup is set to class name the instance of
	 * this class is created (should be subclass of ThreadGroup)
	 * and EventDispatchThread is created inside of it
	 * 
	 * If loaded class overrides uncaughtException instance
	 * handles all uncaught exception on EventDispatchThread
	 */
	ThreadGroup threadGroup = null;
        String tgName = System.getProperty("awt.threadgroup", "");

	if (tgName.length() != 0) {
	    try {
		Constructor ctor = Class.forName(tgName).
		    getConstructor(new Class[] {String.class});
		threadGroup = (ThreadGroup)ctor.newInstance(new Object[] {"AWT-ThreadGroup"});
	    } catch (Exception e) {
		System.err.println("Failed loading " + tgName + ": " + e);
	    }
	}

	Runnable initEQ = new Runnable() {
	    public void run () {
		EventQueue eventQueue;

		String eqName = Toolkit.getProperty("AWT.EventQueueClass",
						    "java.awt.EventQueue");
		
		try {
		    eventQueue = (EventQueue)Class.forName(eqName).newInstance();
		} catch (Exception e) {
		    System.err.println("Failed loading " + eqName + ": " + e);
		    eventQueue = new EventQueue();
		}
		AppContext appContext = AppContext.getAppContext();
		appContext.put(AppContext.EVENT_QUEUE_KEY, eventQueue);

		PostEventQueue postEventQueue = new PostEventQueue(eventQueue);
		appContext.put(POST_EVENT_QUEUE_KEY, postEventQueue);
	    }
	};

	if (threadGroup != null) {
	    Thread eqInitThread = new Thread(threadGroup, initEQ, "EventQueue-Init");
	    eqInitThread.start();
	    try {
		eqInitThread.join();
	    } catch (InterruptedException e) {
		e.printStackTrace();
	    }
	} else {
	    initEQ.run();
	}
    }

    public abstract WindowPeer createWindow(Window target)
        throws HeadlessException;

    public abstract FramePeer createFrame(Frame target)
        throws HeadlessException;

    public abstract DialogPeer createDialog(Dialog target)
        throws HeadlessException;

    public abstract ButtonPeer createButton(Button target)
        throws HeadlessException;

    public abstract TextFieldPeer createTextField(TextField target)
        throws HeadlessException;

    public abstract ChoicePeer createChoice(Choice target)
        throws HeadlessException;

    public abstract LabelPeer createLabel(Label target)
        throws HeadlessException;

    public abstract ListPeer createList(java.awt.List target)
        throws HeadlessException;

    public abstract CheckboxPeer createCheckbox(Checkbox target)
        throws HeadlessException;

    public abstract ScrollbarPeer createScrollbar(Scrollbar target)
        throws HeadlessException;

    public abstract ScrollPanePeer createScrollPane(ScrollPane target)
        throws HeadlessException;

    public abstract TextAreaPeer createTextArea(TextArea target)
        throws HeadlessException;

    public abstract FileDialogPeer createFileDialog(FileDialog target)
        throws HeadlessException;

    public abstract MenuBarPeer createMenuBar(MenuBar target)
        throws HeadlessException;

    public abstract MenuPeer createMenu(Menu target)
        throws HeadlessException;

    public abstract PopupMenuPeer createPopupMenu(PopupMenu target)
        throws HeadlessException;

    public abstract MenuItemPeer createMenuItem(MenuItem target)
        throws HeadlessException;

    public abstract CheckboxMenuItemPeer createCheckboxMenuItem(
        CheckboxMenuItem target)
        throws HeadlessException;

    public abstract DragSourceContextPeer createDragSourceContextPeer(
        DragGestureEvent dge)
        throws InvalidDnDOperationException;

    public abstract FontPeer getFontPeer(String name, int style);

    public abstract RobotPeer createRobot(Robot target, GraphicsDevice screen)
        throws AWTException;

    public KeyboardFocusManagerPeer createKeyboardFocusManagerPeer(KeyboardFocusManager manager) throws HeadlessException {
        KeyboardFocusManagerPeerImpl peer = new KeyboardFocusManagerPeerImpl(manager);
        return peer;
    }

    /*
     * Create a new AppContext, along with its EventQueue, for a
     * new ThreadGroup.  Browser code, for example, would use this
     * method to create an AppContext & EventQueue for an Applet.
     */
    public static AppContext createNewAppContext() {
	ThreadGroup threadGroup = Thread.currentThread().getThreadGroup();
	EventQueue eventQueue;
        String eqName = Toolkit.getProperty("AWT.EventQueueClass",
                                            "java.awt.EventQueue");
        try {
            eventQueue = (EventQueue)Class.forName(eqName).newInstance();
        } catch (Exception e) {
            System.err.println("Failed loading " + eqName + ": " + e);
            eventQueue = new EventQueue();
        }
	AppContext appContext = new AppContext(threadGroup);
	appContext.put(AppContext.EVENT_QUEUE_KEY, eventQueue);

	PostEventQueue postEventQueue = new PostEventQueue(eventQueue);
	appContext.put(POST_EVENT_QUEUE_KEY, postEventQueue);

	return appContext;
    }

    private static  Object getPrivateKey(final Object o){
        Object retObj = null;
        if (componentKeyField == null || menuComponentKeyField == null){
            AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        try {
                            componentKeyField = Component.class.getDeclaredField("privateKey");
                            menuComponentKeyField = MenuComponent.class.getDeclaredField("privateKey");
                            if (menuComponentKeyField != null){
                                menuComponentKeyField.setAccessible(true);
                            }
                            if (componentKeyField != null){
                                componentKeyField.setAccessible(true);
                            }
                        } catch (SecurityException e) {
                            assert false;
                        } catch (NoSuchFieldException e) {
                            assert false;
                        }
                        return null;
                    }//run
                });
        }
        try{
            if (o instanceof Component){
                retObj = componentKeyField.get(o);
            }else{
                if (o instanceof MenuComponent){
                    retObj = menuComponentKeyField.get(o);
                }
            }
        } catch( IllegalAccessException e){
            assert false;
        }
        return retObj;
    }

    static void wakeupEventQueue(EventQueue q, boolean isShutdown){
        if (wakeupMethod == null){
            wakeupMethod = (Method)AccessController.doPrivileged(new PrivilegedAction(){
                    public Object run(){
                        try {
                            Method method  = EventQueue.class.getDeclaredMethod("wakeup",new Class [] {Boolean.TYPE} );
                            if (method != null) {
                                method.setAccessible(true);
                            }
                            return method;
                        } catch (NoSuchMethodException e) {
                            assert false;
                        } catch (SecurityException e) {
                            assert false;
                        }
                        return null;
                    }//run
                });
        }
        try{
            if (wakeupMethod != null){
                wakeupMethod.invoke(q, new Object[]{Boolean.valueOf(isShutdown)});
            }
        } catch (InvocationTargetException e){
            assert false;
        } catch (IllegalAccessException e) {
            assert false;
        }
    }
    // mapping of components to peers, Hashtable<Component,Peer>
    protected static final Hashtable peerMap = 
        AWTAutoShutdown.getInstance().getPeerMap();  

    /*
     * Fetch the peer associated with the given target (as specified
     * in the peer creation method).  This can be used to determine 
     * things like what the parent peer is.  If the target is null
     * or the target can't be found (either because the a peer was
     * never created for it or the peer was disposed), a null will
     * be returned.
     */
    protected static Object targetToPeer(Object target) {
	if (target != null && !GraphicsEnvironment.isHeadless()) {
	    return peerMap.get(getPrivateKey(target));
	}
	return null;
    }

    protected static void targetCreatedPeer(Object target, Object peer) {
        if (target != null && peer != null &&
            !GraphicsEnvironment.isHeadless()) {
            peerMap.put(getPrivateKey(target), peer);
        }
    }

    protected static void targetDisposedPeer(Object target, Object peer) {
	if (target != null && peer != null &&
            !GraphicsEnvironment.isHeadless()) {
            Object key = getPrivateKey(target); 
	    if (peerMap.get(key) == peer) {
		peerMap.remove(key);
	    }
	}
    }

    // Maps from non-Component/MenuComponent to AppContext.
    // WeakHashMap<Component,AppContext>
    private static final Map appContextMap = 
        Collections.synchronizedMap(new WeakHashMap());


    /**
     * Sets the appContext field of target. If target is not a Component or
     * MenuComponent, this returns false.
     */
    private static native boolean setAppContext(Object target,
                                                 AppContext context);

    /**
     * Returns the appContext field for target. If target is not a
     * Component or MenuComponent this returns null.
     */
    private static native AppContext getAppContext(Object target);

    /*
     * Fetch the AppContext associated with the given target.
     * This can be used to determine things like which EventQueue
     * to use for posting events to a Component.  If the target is
     * null or the target can't be found, a null with be returned.
     */
    public static AppContext targetToAppContext(Object target) {
        if (target == null || GraphicsEnvironment.isHeadless()) {
            return null;
        }
        AppContext context = getAppContext(target);
        if (context == null) {
            // target is not a Component/MenuComponent, try the
            // appContextMap.
            context = (AppContext)appContextMap.get(target);
        }
        return context;
    }

     /**
      * Sets the synchronous status of focus requests on lightweight
      * components in the specified window to the specified value.
      * If the boolean parameter is <code>true</code> then the focus
      * requests on lightweight components will be performed
      * synchronously, if it is <code>false</code>, then asynchronously.
      * By default, all windows have their lightweight request status
      * set to asynchronous.
      * <p>
      * The application can only set the status of lightweight focus
      * requests to synchronous for any of its windows if it doesn't
      * perform focus transfers between different heavyweight containers.
      * In this case the observable focus behaviour is the same as with
      * asynchronous status.
      * <p>
      * If the application performs focus transfer between different
      * heavyweight containers and sets the lightweight focus request
      * status to synchronous for any of its windows, then further focus
      * behaviour is unspecified.
      * <p>
      * @param    w window for which the lightweight focus request status
      *             should be set
      * @param    status the value of lightweight focus request status
      */

    public static void setLWRequestStatus(Window changed,boolean status){
        if (syncLWRequestsField == null){
            syncLWRequestsField = (Field)AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        try {
                            Field field  = Window.class.getDeclaredField("syncLWRequests");
                            if (field != null) {
                                field.setAccessible(true);
                            }
                            return field;
                        } catch (NoSuchFieldException e) {
                            assert false;
                        } catch (SecurityException e) {
                            assert false;
                        }
                        return null;
                    }//run
                });
        }
        try{
            if (syncLWRequestsField != null){
                syncLWRequestsField.setBoolean(changed, status);
            }
        } catch( IllegalAccessException e){
            assert false;
        }
    }; 

    public static void checkAndSetPolicy(Container cont, boolean isSwingCont)
    {
        FocusTraversalPolicy defaultPolicy = KeyboardFocusManager
            .getCurrentKeyboardFocusManager().getDefaultFocusTraversalPolicy();

        String toolkitName = Toolkit.getDefaultToolkit().getClass().getName();
        // if this is not XAWT then use default policy
        // because Swing change it
        if (!"sun.awt.X11.XToolkit".equals(toolkitName)) {
            cont.setFocusTraversalPolicy(defaultPolicy);
            return;
        }

        String policyName = defaultPolicy.getClass().getName();

        if (DefaultFocusTraversalPolicy.class != defaultPolicy.getClass()) {
            // Policy was changed
            // Check if it is awt policy or swing policy
            // If it is Swing policy we shouldn't use it in AWT frames
            // If it is AWT policy  we shouldn't use it in Swing frames
            // Otherwise we should use this policy
            if (policyName.startsWith("java.awt.")) {
                // AWT
                if (isSwingCont) {
                    // Can't use AWT policy in Swing windows - should use Swing's one.
                    defaultPolicy = createLayoutPolicy();
                } else {
                    // New awt policy.
                }
            } else if (policyName.startsWith("javax.swing.")) {
                if (isSwingCont) {
                    // New Swing's policy
                } else {
                    defaultPolicy = new DefaultFocusTraversalPolicy();
                }
            }
        } else {
            // Policy is default, use different default policy for swing
            if (isSwingCont) {
                defaultPolicy = createLayoutPolicy();
            }
        }
        cont.setFocusTraversalPolicy(defaultPolicy);
    }

    private static FocusTraversalPolicy createLayoutPolicy() {
        FocusTraversalPolicy policy = null;
        try {
            Class layoutPolicyClass = 
                Class.forName("javax.swing.LayoutFocusTraversalPolicy");
            policy = (FocusTraversalPolicy) layoutPolicyClass.newInstance();
        }
        catch (ClassNotFoundException e) {
            assert false;
        }
        catch (InstantiationException e) {
            assert false;
        }
        catch (IllegalAccessException e) {
            assert false;
        }

        return policy;
    }

    /*
     * Insert a mapping from target to AppContext, for later retrieval
     * via targetToAppContext() above.
     */
    public static void insertTargetMapping(Object target, AppContext appContext) {
        if (!GraphicsEnvironment.isHeadless()) {
            if (!setAppContext(target, appContext)) {
                // Target is not a Component/MenuComponent, use the private Map
                // instead.
                appContextMap.put(target, appContext);
            }
        }
    }

    /*
     * Post an AWTEvent to the Java EventQueue, using the PostEventQueue
     * to avoid possibly calling client code (EventQueueSubclass.postEvent())
     * on the toolkit (AWT-Windows/AWT-Motif) thread.  This function should 
     * not be called under another lock since it locks the EventQueue.   
     * See bugids 4632918, 4526597.  
     */
    public static void postEvent(AppContext appContext, AWTEvent event) {
	if (event == null) {
	    throw new NullPointerException();
	}
        AppContext eventContext = targetToAppContext(event.getSource());
        if (eventContext != null && !eventContext.equals(appContext)) {
            log.fine("Event posted on wrong app context : " + event);
        }
	PostEventQueue postEventQueue =
	    (PostEventQueue)appContext.get(POST_EVENT_QUEUE_KEY);
        if(postEventQueue != null) {
            postEventQueue.postEvent(event);
        }
    }

    /*
     * Flush any pending events which haven't been posted to the AWT
     * EventQueue yet.
     */
    public static void flushPendingEvents()  {
	AppContext appContext = AppContext.getAppContext();
	PostEventQueue postEventQueue =
	    (PostEventQueue)appContext.get(POST_EVENT_QUEUE_KEY);
        if(postEventQueue != null) {
            postEventQueue.flush();
        }
    }

    public static boolean isPostEventQueueEmpty()  {
        AppContext appContext = AppContext.getAppContext();
        PostEventQueue postEventQueue =
            (PostEventQueue)appContext.get(POST_EVENT_QUEUE_KEY);
        if (postEventQueue != null) {
            return postEventQueue.noEvents();
        } else {
            return true;
        }
    }

    /*
     * Execute a chunk of code on the Java event handler thread for the
     * given target.  Does not wait for the execution to occur before
     * returning to the caller.
     */
    public static void executeOnEventHandlerThread(Object target,
						   Runnable runnable) {
        executeOnEventHandlerThread(new PeerEvent(target, runnable, PeerEvent.PRIORITY_EVENT));
    }

    /*
     * Execute a chunk of code on the Java event handler thread for the
     * given target.  Does not wait for the execution to occur before
     * returning to the caller.
     */
    public static void executeOnEventHandlerThread(PeerEvent peerEvent) {
        postEvent(targetToAppContext(peerEvent.getSource()), peerEvent);
    }

    public Dimension getScreenSize() {
	return new Dimension(getScreenWidth(), getScreenHeight());
    }
    protected abstract int getScreenWidth();
    protected abstract int getScreenHeight();

    public static final FontMetrics[] lastMetrics = new FontMetrics[5];

    public FontMetrics getFontMetrics(Font font) {
	for (int i = 0; i < lastMetrics.length; i++) {
	    FontMetrics lm = lastMetrics[i];
	    if (lm == null) {
		break;
	    }
	    if (lm.getFont() == font) {
		return lm;
	    }
	}
	FontMetrics lm = new FontDesignMetrics(font);
	System.arraycopy(lastMetrics, 0, lastMetrics, 1, lastMetrics.length-1);
	lastMetrics[0] = lm;
	return lm;
    }
    
    public String[] getFontList() {
	String[] hardwiredFontList = {
	    "Dialog", "SansSerif", "Serif", "Monospaced", "DialogInput"

	    // -- Obsolete font names from 1.0.2.  It was decided that
	    // -- getFontList should not return these old names:
	    //    "Helvetica", "TimesRoman", "Courier", "ZapfDingbats"
	};
	return hardwiredFontList;
    }

    public PanelPeer createPanel(Panel target) {
        return (PanelPeer)createComponent(target);
    }

    public CanvasPeer createCanvas(Canvas target) {
        return (CanvasPeer)createComponent(target);
    }

    static SoftCache imgCache = new SoftCache();

    static synchronized Image getImageFromHash(Toolkit tk, URL url) {
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    try {
		java.security.Permission perm = 
		    url.openConnection().getPermission();
		if (perm != null) {
		    try {
			sm.checkPermission(perm);
		    } catch (SecurityException se) {
			// fallback to checkRead/checkConnect for pre 1.2
			// security managers
			if ((perm instanceof java.io.FilePermission) &&
			    perm.getActions().indexOf("read") != -1) {
			    sm.checkRead(perm.getName());
			} else if ((perm instanceof 
			    java.net.SocketPermission) &&
			    perm.getActions().indexOf("connect") != -1) {
			    sm.checkConnect(url.getHost(), url.getPort());
			} else {
			    throw se;
			}
		    }
		}
	    } catch (java.io.IOException ioe) {
		    sm.checkConnect(url.getHost(), url.getPort());
	    }
	}
	Image img = (Image)imgCache.get(url);
	if (img == null) {
	    try {	
		img = tk.createImage(new URLImageSource(url));
		imgCache.put(url, img);
	    } catch (Exception e) {
	    }
	}
	return img;
    }

    static synchronized Image getImageFromHash(Toolkit tk,
                                               String filename) {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkRead(filename);
	}
	Image img = (Image)imgCache.get(filename);
	if (img == null) {
	    try {	
		img = tk.createImage(new FileImageSource(filename));
		imgCache.put(filename, img);
	    } catch (Exception e) {
	    }
	}
	return img;
    }

    public Image getImage(String filename) {
	return getImageFromHash(this, filename);
    }

    public Image getImage(URL url) {
	return getImageFromHash(this, url);
    }

    public Image createImage(String filename) {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkRead(filename);
	}
	return createImage(new FileImageSource(filename));
    }

    public Image createImage(URL url) {
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    try {
		java.security.Permission perm = 
		    url.openConnection().getPermission();
		if (perm != null) {
		    try {
			sm.checkPermission(perm);
		    } catch (SecurityException se) {
			// fallback to checkRead/checkConnect for pre 1.2
			// security managers
			if ((perm instanceof java.io.FilePermission) &&
			    perm.getActions().indexOf("read") != -1) {
			    sm.checkRead(perm.getName());
			} else if ((perm instanceof 
			    java.net.SocketPermission) &&
			    perm.getActions().indexOf("connect") != -1) {
			    sm.checkConnect(url.getHost(), url.getPort());
			} else {
			    throw se;
			}
		    }
		}
	    } catch (java.io.IOException ioe) {
		    sm.checkConnect(url.getHost(), url.getPort());
	    }
	}
	return createImage(new URLImageSource(url));
    }

    public Image createImage(byte[] data, int offset, int length) {
	return createImage(new ByteArrayImageSource(data, offset, length));
    }

    public Image createImage(ImageProducer producer) {
        return new ToolkitImage(producer);
    }

    public int checkImage(Image img, int w, int h, ImageObserver o) {
        if (!(img instanceof ToolkitImage)) {
            return ImageObserver.ALLBITS;
        }
            
        ToolkitImage tkimg = (ToolkitImage)img;
	int repbits;
	if (w == 0 || h == 0) {
	    repbits = ImageObserver.ALLBITS;
	} else {
	    repbits = tkimg.getImageRep().check(o);
	}
	return tkimg.check(o) | repbits;
    }

    public boolean prepareImage(Image img, int w, int h, ImageObserver o) {
	if (w == 0 || h == 0) {
	    return true;
	}

        // Must be a ToolkitImage
        if (!(img instanceof ToolkitImage)) {
            return true;
        }

        ToolkitImage tkimg = (ToolkitImage)img;
	if (tkimg.hasError()) {
	    if (o != null) {
		o.imageUpdate(img, ImageObserver.ERROR|ImageObserver.ABORT,
			      -1, -1, -1, -1);
	    }
	    return false;
	}
	ImageRepresentation ir = tkimg.getImageRep();
	return ir.prepare(o);
    }

    protected EventQueue getSystemEventQueueImpl() {
        return getSystemEventQueueImplPP();
    }

    // Package private implementation
    static EventQueue getSystemEventQueueImplPP() {
        AppContext appContext = AppContext.getAppContext();
        EventQueue theEventQueue =
            (EventQueue)appContext.get(AppContext.EVENT_QUEUE_KEY);
        return theEventQueue;
    }

    /**
     * Give native peers the ability to query the native container 
     * given a native component (eg the direct parent may be lightweight).
     */
    public static Container getNativeContainer(Component c) {
	return Toolkit.getNativeContainer(c);
    }

    /**
     * Returns a new input method window, with behavior as specified in
     * {@link java.awt.im.spi.InputMethodContext#createInputMethodWindow}.
     * If the inputContext is not null, the window should return it from its
     * getInputContext() method. The window needs to implement
     * sun.awt.im.InputMethodWindow.
     * <p>
     * SunToolkit subclasses can override this method to return better input
     * method windows.
     */
    public Window createInputMethodWindow(String title, InputContext context) {
        return new sun.awt.im.SimpleInputMethodWindow(title, context);
    }

    /**
     * Returns whether enableInputMethods should be set to true for peered
     * TextComponent instances on this platform. False by default.
     */
    public boolean enableInputMethodsForTextComponent() {
        return false;
    }
    
    private static Locale startupLocale = null;
    
    /**
     * Returns the locale in which the runtime was started.
     */
    public static Locale getStartupLocale() {
        if (startupLocale == null) {
            String language, region, country, variant;
            language = (String) AccessController.doPrivileged(
                            new GetPropertyAction("user.language", "en"));
            // for compatibility, check for old user.region property
            region = (String) AccessController.doPrivileged(
                            new GetPropertyAction("user.region"));
            if (region != null) {
                // region can be of form country, country_variant, or _variant
                int i = region.indexOf('_');
                if (i >= 0) {
                    country = region.substring(0, i);
                    variant = region.substring(i + 1);
                } else {
                    country = region;
                    variant = "";
                }
            } else {
                country = (String) AccessController.doPrivileged(
                                new GetPropertyAction("user.country", ""));
                variant = (String) AccessController.doPrivileged(
                                new GetPropertyAction("user.variant", ""));
            }
            startupLocale = new Locale(language, country, variant);
        }
        return startupLocale;
    }

    /**
     * Returns the default keyboard locale of the underlying operating system
     */
    public Locale getDefaultKeyboardLocale() {
        return getStartupLocale();
    }

    private static String dataTransfererClassName = null;

    protected static void setDataTransfererClassName(String className) {
        dataTransfererClassName = className;
    }

    public static String getDataTransfererClassName() {
        if (dataTransfererClassName == null) {
            Toolkit.getDefaultToolkit(); // transferer set during toolkit init
        }
        return dataTransfererClassName;
    }

    // Support for window closing event notifications
    private transient WindowClosingListener windowClosingListener = null;
    /**
     * @see sun.awt.WindowClosingSupport#getWindowClosingListener
     */
    public WindowClosingListener getWindowClosingListener() {
        return windowClosingListener;
    }
    /**
     * @see sun.awt.WindowClosingSupport#setWindowClosingListener
     */
    public void setWindowClosingListener(WindowClosingListener wcl) {
        windowClosingListener = wcl;
    }

    /**
     * @see sun.awt.WindowClosingListener#windowClosingNotify
     */
    public RuntimeException windowClosingNotify(WindowEvent event) {
        if (windowClosingListener != null) {
            return windowClosingListener.windowClosingNotify(event);
        } else {
            return null;
        }
    }
    /**
     * @see sun.awt.WindowClosingListener#windowClosingDelivered
     */
    public RuntimeException windowClosingDelivered(WindowEvent event) {
        if (windowClosingListener != null) {
            return windowClosingListener.windowClosingDelivered(event);
        } else {
            return null;
        }
    }

    private static DefaultMouseInfoPeer mPeer = null;

    protected synchronized MouseInfoPeer getMouseInfoPeer() {
        if (mPeer == null) {
            mPeer = new DefaultMouseInfoPeer();
        }
        return mPeer;
    }
    

    /**
     * Returns whether default toolkit needs the support of the xembed 
     * from embedding host(if any).
     * @return <code>true</code>, if XEmbed is needed, <code>false</code> otherwise
     */
    public static boolean needsXEmbed() {
        String noxembed = (String) AccessController.
            doPrivileged(new GetPropertyAction("sun.awt.noxembed", "false"));
        if ("true".equals(noxembed)) {
            return false;
        }

        Toolkit tk = Toolkit.getDefaultToolkit();
        if (tk instanceof SunToolkit) {
            // SunToolkit descendants should override this method to specify
            // concrete behavior
            return ((SunToolkit)tk).needsXEmbedImpl();
        } else {
            // Non-SunToolkit doubtly might support XEmbed
            return false;
        }
    }

    /**
     * Returns whether this toolkit needs the support of the xembed 
     * from embedding host(if any).
     * @return <code>true</code>, if XEmbed is needed, <code>false</code> otherwise
     */
    protected boolean needsXEmbedImpl() {
        return false;
    }
    
    public static boolean isLightweightOrUnknown(Component comp) {
        if (comp.isLightweight()
            || !(getDefaultToolkit() instanceof SunToolkit)) 
        {
            return true;
        }
        return !(comp instanceof Button
            || comp instanceof Canvas
            || comp instanceof Checkbox
            || comp instanceof Choice
            || comp instanceof Label
            || comp instanceof java.awt.List
            || comp instanceof Panel
            || comp instanceof Scrollbar
            || comp instanceof ScrollPane
            || comp instanceof TextArea
            || comp instanceof TextField
            || comp instanceof Window);
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


} // class SunToolkit



/*
 * PostEventQueue is a Thread that runs in the same AppContext as the
 * Java EventQueue.  It is a queue of AWTEvents to be posted to the
 * Java EventQueue.  The toolkit Thread (AWT-Windows/AWT-Motif) posts
 * events to this queue, which then calls EventQueue.postEvent().
 *
 * We do this because EventQueue.postEvent() may be overridden by client
 * code, and we mustn't ever call client code from the toolkit thread.
 */
class PostEventQueue {
    private EventQueueItem queueHead = null;
    private EventQueueItem queueTail = null;
    private final EventQueue eventQueue;

    PostEventQueue(EventQueue eq) {
	eventQueue = eq;
    }

    public boolean noEvents() {
        return queueHead == null;
    }

    /*
     * Continually post pending AWTEvents to the Java EventQueue.
     */
    public void flush() {
        if (queueHead != null) {
            EventQueueItem tempQueue;
            /*
             * We have to execute the loop inside the synchronized block
             * to ensure that the flush is completed before a new event
             * can be posted to this queue.
             */
            synchronized (this) {
                tempQueue = queueHead;
                queueHead = queueTail = null;
                /*
                 * If this PostEventQueue is flushed in parallel on two
                 * different threads tempQueue will be null for one of them.
                 */
                while (tempQueue != null) {
                    eventQueue.postEvent(tempQueue.event);
                    tempQueue = tempQueue.next;
                }
            }
        }
    }

    /*
     * Enqueue an AWTEvent to be posted to the Java EventQueue.
     */
    void postEvent(AWTEvent event) {
	EventQueueItem item = new EventQueueItem(event);

        synchronized (this) {
            if (queueHead == null) {
                queueHead = queueTail = item;
            } else {
                queueTail.next = item;
                queueTail = item;
            }
        }
        SunToolkit.wakeupEventQueue(eventQueue, event.getSource() == AWTAutoShutdown.getInstance());
    }
} // class PostEventQueue

class EventQueueItem {
    AWTEvent event;
    EventQueueItem next;

    EventQueueItem(AWTEvent evt) {
        event = evt;
    }
} // class EventQueueItem
