/*
 * @(#)MNetscapePluginObject.java	1.53 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.viewer;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Component;
import java.awt.Panel;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.EventQueue;
import java.awt.Dimension;
import java.awt.event.WindowEvent;
import java.io.PrintStream;
import java.net.URL;
import java.util.Iterator;
import java.util.Set;
import com.sun.deploy.perf.DeployPerfUtil;
import sun.awt.SunToolkit;
import sun.awt.AppContext;
import sun.awt.EmbeddedFrame;
import sun.applet.AppletClassLoader;
import sun.applet.AppletListener;
import sun.applet.AppletEvent;
import sun.applet.AppletPanel;
import sun.plugin.BeansApplet;
import sun.plugin.AppletViewer;
import sun.plugin.BeansViewer;
import sun.plugin.ClassLoaderInfo;
import sun.plugin.navig.motif.Plugin;
import sun.plugin.navig.motif.Worker;
import sun.plugin.util.NotifierObject;
import sun.plugin.viewer.frame.MNetscapeEmbeddedFrame;
import sun.plugin.viewer.frame.XNetscapeEmbeddedFrame;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.viewer.context.NetscapeAppletContext;
import sun.plugin.services.BrowserService;
import sun.print.PSPrinterJob;
import com.sun.deploy.util.DeployAWTUtil;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/*
 * Panel for each applet running in the Java Plugin VM.  This class
 * encapsulates the viewing of the Applet. It extends AppletViewer,
 * which extends AppletPanel, which provides the basic state machine
 * of an applet.  There are 5 threads associated with this class, in
 * addition to any associated with the applet:
 *
 *   The event processing thread (of type MotifAppletViewer)
 *   The class loading thread (also of type MotifAppletViewer,
 *         responsible for creating the class loader etc.)
 *   The init thread (Initer thread) , used for calling init on the applet
 *   The destroy thread (DestroyerThread), used for calling destroy
 *   The Plugin's main thread which runs the applet viewer by calling
 *                setWindow etc.
 *
 * Most of the hard work is done in our baseclasses, sun.plugin.AppletViewer
 * and sun.plugin.AppletPanel.
 *
 * The life-cycle for a MotifAppletpanel is slightly strange and is
 * driven by the Netscape plugin API.
 *
 *    (1) First, netscape creates a new plugin instance using CreateApplet

 *    (2) Typically, the next thing that happens is that Netscape
 *	  sets the X11 window information using JAVA_PLUGIN_WINDOW
 *	  At this point we can create our AWT EmbeddedFrame using
 *	  the parent X11 window ID privided by netscape.
 *
 *    (3) Typically, the next thing that happens will be that the
 *	  plugin figures out our DOCBASE and sends it to us.
 *	  This step may sometimes ocur before (2). JAVA_PLUGIN_DOCBASE
 *	  This allows us to set the DOCBASE in our parent.
 *
 *    (4) When both (2) and (3) have occurred we have both a DOCBASE
 *	  value and an EmbeddedFrame, so we can call init() to 
 *	  actually do all the hard work of starting the applet.
 *
 *    (5) We live a fruitful and industrious life, with our applet
 *	  happily running and painting.
 *
 *    (6) When netscape moves to a different page (or whatever) the
 *	  plugin JAVA_PLUGIN_DESTROY method will be called.
 *
 *	  At this point we are called to destroy the applet.
 * */

/**
 * <p> MNetscapePluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class MNetscapePluginObject implements 
	sun.plugin.AppletStatusListener
{
    /** 
     * <p> Embedded Frame object for the plugin.
     * </p>
     */
    private EmbeddedFrame frame = null;

    /** 
     * <p> Panel object for hosting the applet or bean.
     * </p>
     */
    protected AppletViewer panel = null;

    /** 
     * <p> ID of an applet or bean in Navigator.
     * </p>
     */
    protected int id = -1;

    /** 
      * <p> Parent window id of the embedded frame.
      * </p>
      */
    private int winID = 0;

    /**
      * <p> Width of the embedded frame. This is used by relative dimension support.
      * </p>
      */      
    private int width = 0;

    /**
      * <p> Height of the embedded frame. This is used by relative dimension support.
      * </p>
      */      
    private int height = 0;

    /**
     * <p> Identifier of the applet instance.
     * </p>
     */
    private String identifier = null;

    /**
     * <p> Set when the Toolkit is XToolkit - Motifless AWT
     * </p>
     */
    private boolean use_xtoolkit = false;

    /**
     * <p> Indicate if plugin object has been destroyed .
     * </p>
     */
    private boolean destroyed = false;

    private final Object syncDestroy = new Object();

    private boolean isAppletStoppedOrDestroyed = false;

    /**
     * Handle to the applet class loader, which we create early in
     * order to provoke the creation of the applet's ThreadGroup to be
     * able to create the EmbeddedFrame in the correct AppContext.
     */
    private AppletClassLoader tmpLoader;

    private boolean isBeans = false;

    private boolean isLegacy = false;

    /**
     * <p> Create a MNetscapePluginObject.
     * </p>
     *
     * @param id Identifier of the Plugin instance.
     */
    MNetscapePluginObject(int id, boolean isBeans, String identifier)
    {
        this.id = id;
	this.identifier = identifier;

	// Obtain panel from LifeCycleManager
	this.panel = LifeCycleManager.getAppletPanel(identifier);

	use_xtoolkit = (Toolkit.getDefaultToolkit() instanceof sun.awt.X11.XToolkit);

        this.isBeans = isBeans;

	if (isBeans == false)
	{
	    // Applet case
	    if (this.panel == null) {
	        this.panel = new AppletViewer();
	    }

	    // Obtain the applet context and reset the handle
	    PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();
	    
	    if (pac == null)
	    {
		BrowserService bs = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
		pac = (PluginAppletContext) bs.getAppletContext();
	    }

	    pac.setAppletContextHandle(id);
	    this.panel.setAppletContext(pac);
	}
	else
	{
	    // Beans case
	    if (this.panel == null)
		this.panel = new BeansViewer();

	    // Obtain the applet context and reset the handle
	    PluginBeansContext pbc = (PluginBeansContext) this.panel.getAppletContext();

	    if (pbc == null)
	    {
		BrowserService bs = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
		pbc = (PluginBeansContext) bs.getBeansContext();
	    }

	    pbc.setAppletContextHandle(id);
	    this.panel.setAppletContext(pbc);
	}

        this.isLegacy = this.panel.isLegacyLifeCycle();
    }

    // Wait with timeout to avoid locking up the browser
    public static void waitForNotification(NotifierObject obj,
                                           long timeoutInMilliseconds) {
        long startTime = System.currentTimeMillis();
        while (!obj.getNotified() &&
                   (System.currentTimeMillis() - startTime < timeoutInMilliseconds)) {
            // Don't consume all CPU while waiting
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
            }
        }
    }

    /**
     * <p> Create the embedded frame for an applet or beans depends on the 
     * runtime type.
     * </p>
     */
    private EmbeddedFrame createFrame(final int handle, final int xembed)  
    {
        DeployPerfUtil.put("START - Java   - ENV - create embedded browser frame (Mozilla:UNIX)");

        EmbeddedFrame  frame;

        //get ClassLoader, ThreadGroup and AppContext for the applet
        tmpLoader = panel.getAppletClassLoader();

        if (tmpLoader == null) 
        {
            if (use_xtoolkit) 
            {
                frame = new XNetscapeEmbeddedFrame((long) handle, xembed != 0);
            } 
            else 
            {
                frame = new MNetscapeEmbeddedFrame(handle, xembed != 0);
            } 
        } 
        else 
        { 
            final EmbeddedFrame[] box = new EmbeddedFrame[1];
            final NotifierObject notifier = new NotifierObject();

            try 
            { 
                Runnable r = new Runnable() 
                { 
                    public void run() 
                    { 
                        try {
                            if (destroyed) return;

                            if (use_xtoolkit) 
                            {
                                box[0] = new XNetscapeEmbeddedFrame((long) handle, xembed != 0);
                            } 
                            else 
                            {
                                box[0] = new MNetscapeEmbeddedFrame(handle, xembed != 0);
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        } finally {
                            notifier.setNotified();
                        }
                    }
                };
                DeployAWTUtil.invokeLater(tmpLoader.getAppContext(), r);

                waitForNotification(notifier, 5000);
                if (!notifier.getNotified()) {
                    throw new RuntimeException("Error while creating embedded frame");
                }
            } 
            catch (Exception e) 
            {
                e.printStackTrace();
            }

            frame = box[0];
        }

        DeployPerfUtil.put("END   - Java   - ENV - create embedded browser frame (Mozilla:UNIX)");

	return frame;
    }

    /**
     * Our X11 window information has changed.
     * Newid is the new X11 window ID of our plugin window in Navigator.
     * Width, height, x, and y provide more info on that window.
     * Set window is only called by the plugin, and there is only one thread
     * that can do this, so this need not be synchronized
     */
    public void setWindow(int newID, int xembed, int width, 
						  int height, int x, int y) {

	// setWindow() is called whenever the plug-in is started,
	// stoped, and resized. When the plug-in first staryts
	// up before Applet.start() is called, setWindow()
	// will be called with a valid plug-in window handle.
	// When the plug-in is destroyed, Applet.stop() will
	// be called, and followed by a setWindow() with a 
	// NULL plug-in window handle.


    // Frame has been created
	// Check to make sure the same winID doesn't exist.  Multiple
	// setWindow() for the same winID may mislocate SubstructureRedirectMask
    // for the embedder and its client(s) in XEmbed.
		if (width == 0) width = 1;
		if (height == 0) height = 1;

	if (winID == newID) {
	    if (width != this.width || height != this.height)
		setFrameSize(width, height);
            return;
	}

        // A different handle is passed. Remove existing frame.
	if (frame != null)
	    destroyFrame();

	frame = null;
	winID = newID;
	Trace.println("New window ID: " + Integer.toHexString(winID), TraceLevel.BASIC);

	if (winID != 0)
	{
	    // Create a Frame for the applet.
	    // Regardless of whether it's XToolkit or Motif, plugin only
	    // needs to pass the X window handle provided by Netscape to AWT.
	    // In the case of Motif, MNetscapeEmbeddedFrame will take care
	    // of creating the manageable child widget base for the Motif frame.
	    Trace.println("Value of xembed: " + xembed, TraceLevel.BASIC);
            frame = createFrame(winID, xembed);

	    if (frame == null) {
		System.err.println("Creation of frame failed");
		return;
	    }

	    //	    frame.setLayout(null);
	    // Needed for accessibility stuff. There is some optimization
	    // in the accessibility mechanism so that it only pays attention
	    // to frames with window listeners. Ref: Peter Korn for details.
	    // frame.addWindowListener(this);
	    // frame.setVisible(true);
	    

	    Object o = getJavaObject();
	    Applet applet;
	    // If the applet has not been set, attempt to set its dimensions.
	    // Try to read the parameters as specified in the html tags
	    // and stored in width and height. If those are not integers,
	    // then just use the width, height provided by the browser
	    // in this setwindow call.
	    // The applet is normally set in the createApplet call
	    if (o == null) {
		Trace.println("setWindow: call before applet exists:" + Integer.toHexString(winID), TraceLevel.BASIC); 
		try {
		    int wd = Integer.parseInt(getParameter("width"));
		    int ht = Integer.parseInt(getParameter("height"));
		    this.width = wd;
		    this.height = ht;
		} catch (NumberFormatException e) {
		    // Try and maintain consistency between the parameters and
		    // our width/height. If the parameters are not properly
		    // set to be integers, then reset them to our w, h
		    setParameter("width", new Integer(width));
		    setParameter("height", new Integer(height));
		    this.width = width;
		    this.height = height;
		}
	    } else {
		// The applet exists, so resize it and its frame, and
		// have the width/height parameters reflect these values
		setParameter("width", new Integer(width));
		setParameter("height", new Integer(height));
		this.width = width;
		this.height = height;

		panel.setSize(width, height);
		if (o instanceof Applet) {
		    applet = (Applet) o;
		} else {
		    Trace.println("setWindow: Viewing a bean component", TraceLevel.BASIC);
		    Component c = (Component) o;
		    c.setSize(width, height);
		    applet = (Applet) c.getParent();
		}
		applet.resize(width, height);
		applet.setVisible(true);
	    }

	    /* Note: 2/3/99 - Ben
	       Be very careful with the order in which the window events are called.
	       frame.setbounds changes the size of the window, resulting in
	       xevents being generated that trigger the innerCanvasEH in awt_Frame.c
	       If the XtWidget has not yet been realized, this may result in the event
	       being dropped, and the widget does not get sized properly.
	       The XtWidgets are only realized when the embedded frame is shown.
	       It does not help to have the setbounds be done twice - if the bounds are the
	       same in both calls, the AWT widget will not do anything.
	       
	       The Xt Widget hierarchy for the trivial applet that does nothing is:
	       
	       widget=VendorShell level=5 name=AWTapp wd=100 ht=100
	       widget=XmForm level=4 name=main wd=100 ht=100
	       widget=XmDrawingArea level=3 name=frame_wrap wd=100 ht=100
	       widget=XmDrawingArea level=2 name=frame_canvas wd=100 ht=100
	       widget=XmDrawingArea level=1 name=canvas wd=100 ht=100
	       widget=XmDrawingArea level=0 name=canvas wd=100 ht=100
	       
	       The width's and heights are, of course, from a particular run, but the point
	       is that they are all the same.

	       Something else that might be causing problems is the fact that the
	       EmbeddedFrame is reparented to itself (!) in awt_Frame_MEmbeddedFrame_pShow
	       which overrides the standard pShow (both args to the XReparent are 
	       the same widget).  
	    */

	    panel.setBounds(0, 0, width, height);
	    // Add this appletpanel (self) to the frame we just created
	    // so that it will be viewed in the new frame
	    //frame.add(this);
	    /* Realizes the top level widget, which automatically creates the windows of
	       all its children. */
	    frame.setBounds(0, 0, width, height);
	    
	    // If the docbase has already been set, we can init the applet.
	    //	maybeInit();
	    initPlugin();
	    
	    // To trigger the accessibility mechanisms we initiate the event
	    // queue with a synthesized event
	    //	    WindowEvent actev = new WindowEvent(frame, 
	    //				WindowEvent.WINDOW_ACTIVATED);
	    //Toolkit tk = Toolkit.getDefaultToolkit();
	    //EventQueue eq = tk.getSystemEventQueue();
	    //eq.postEvent(actev);
	}
    }

    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer panel;
	MNetscapePluginObject obj;

	Initer(ThreadGroup group, AppletViewer panel, MNetscapePluginObject obj) {
            //make the initer run in the applet thread group
            super(group, "Plugin-Initer");

	    this.panel = panel;
	    this.obj = obj;
  	}
	/**
         * Main body of thread.  Call init on our PluginPanel.
	 */
	public void run() {

	    // This is important to use sync because "initPanel" must be 
	    // completed before the plugin can continue.
	    //
	    LifeCycleManager.loadAppletPanel(panel);

            LifeCycleManager.initAppletPanel(panel);

	    synchronized(obj)
	    {
		obj.initialized = true;

		//6244718, plugin has been destroyed, don't start plugin. 
		if(obj.destroyed){
			return;
		};

		if (obj.startCount > 0)
		{
		    obj.startCount = 0;
		    obj.startPlugin();
		}
		else if (obj.startCount == 0)
		{
		    obj.startPlugin();
		    obj.stopPlugin();
		}
	    }
	}
    }

    /**
     * <p> Add a new panel to the embedded frame and start to download the 
     * applet/bean.
     * </p>
     */
    private synchronized void initPlugin()
    {
	assert (panel != null);

	this.panel.addAppletStatusListener(this);
	// If panel is available, insert it into the frame and
	// start the applet/bean
	if (frame != null)
	{
	    // Fixed #4754451: Applet can have methods running on main
	    // thread event queue. 
	    // 
	    // The cause of this bug is that the frame of the applet 
	    // is created in main thread group. Thus, when certain 
	    // AWT/Swing events are generated, the events will be
	    // dispatched through the wrong event dispatch thread.
	    //
	    // To fix this, we rearrange the AppContext with the frame,
	    // so the proper event queue will be looked up.
	    //
	    // Swing also maintains a Frame list for the AppContext,
	    // so we will have to rearrange it as well.
	    //
	    try {
               /*
                *  Add panel to the frame before proceeding further - 
                *  This is to ensure that applet's area would be displayable
                *  during applet's init() method.
                */
               frame.add(panel);

		// Use invoke later to avoid blocking and race condition
		// with browser window.
		//
		DeployAWTUtil.invokeLater(frame, new Runnable()
		    {
			public void run()
			{			    
			    frame.setVisible(true);
			}
		    });
	    }
	    catch(Exception e)
	    {
	    }

	    // Start an applet or bean
	    if (initialized == false && setDocBase == true)
	    {
		initialized = true;

		// life cycle check. Just for consistence, it is not required for
		// UNIX and LINUX
		LifeCycleManager.checkLifeCycle(panel);

		// This is very important to call panel.initPanel() from a
		// different thread, because it may call back into the plugin
		// for auto config proxy during this call. Thus, it must be 
		// called from a secondary thread. This hack is mainly for JDK 1.2.
		//
                ThreadGroup group = (tmpLoader == null)? null: tmpLoader.getThreadGroup();
		new Initer(group, panel, this).start();
	    }
	}
    }


    /**
     * <p> This count is for remaining the number of pending request for starting 
     * the applet. It is used when the applet is not completely loaded but a 
     * start request is made.
     * </p>
     */
    private int startCount = -1;

    private boolean initialized = false;

    private boolean setDocBase = false;

    /** 
     * <p> Start an applet or bean.
     * </p>
     */
    public synchronized void startPlugin()
    {
	assert (panel != null);

	if (initialized == true)   
	    LifeCycleManager.startAppletPanel(panel);
	else
	{
	    if (startCount < 0)
		startCount = 0;

	    startCount++;
	}
    }



    /** 
     * <p> Stop an applet or bean.
     * </p>
     */
    public synchronized void stopPlugin()
    {
	assert (panel != null);

	if (initialized == true)
	    LifeCycleManager.stopAppletPanel(panel);
	else
	    startCount--;
	
	/* We need to garbage collect any global refence created for applet
	   object to be called by javascript if there is any */
	if (nativeJavaObject != 0) {
	    releaseGlobalRef(nativeJavaObject);
	    nativeJavaObject = 0;
	}
	
    }

    /*
     * <p> Remove the panel from the embedded frame and stop the applet/bean.
     * </p>
     */
    public synchronized void destroyPlugin() 
    {
	assert (panel != null);

        //wait for the Initer thread to finish loading
	panel.waitForLoadingDone(5000);

	PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();

       //set the flag to be false
        isAppletStoppedOrDestroyed = false;

	LifeCycleManager.destroyAppletPanel(identifier, panel);

	//ok let's wait for the applet being destroyed
        try {
            if (panel.getAppletHandlerThread() != null
                  && getLoadingStatus() != AppletPanel.APPLET_ERROR) {
                //wait up to 5 seconds for applet destroy
                synchronized(syncDestroy) {
                     if (!isAppletStoppedOrDestroyed) {
                         syncDestroy.wait(5000);
                     }
                }
            } 
        } catch (Exception e) {
            e.printStackTrace();
        }
		    
	// When applet is stopped/destroyed, it is VERY important to
	// reset the applet context, so applet won't be able to
	// call methods in applet context to interface with the
	// browser if the document has gone away.
	//

	if (pac != null) {
	    pac.setAppletContextHandle(-1);
	    ((NetscapeAppletContext)pac).onClose();
	}

	if (frame != null)
	    destroyFrame();

        //remove applet status listener
        panel.removeAppletStatusListener(null);

        LifeCycleManager.cleanupAppletPanel(panel);
        LifeCycleManager.releaseAppletPanel(panel);

	panel = null;
	frame = null;
	destroyed = true;
    }

    private void signal(int status) {
        switch (status) {
        case AppletPanel.APPLET_STOP:
            if (isLegacy) {
                synchronized (syncDestroy) {
                    isAppletStoppedOrDestroyed = true;
                    syncDestroy.notify();
                }
            }
            break;

        case AppletPanel.APPLET_DESTROY:
        case AppletPanel.APPLET_DISPOSE:
            if (!isLegacy) {
                synchronized (syncDestroy) {
                    isAppletStoppedOrDestroyed = true;
                    syncDestroy.notify();
                }
            }
            break;

        case AppletPanel.APPLET_ERROR:
        case AppletPanel.APPLET_QUIT:
            synchronized(syncDestroy) {
                isAppletStoppedOrDestroyed = true;
                syncDestroy.notify();
            }
        }
    }

    /* 
     * <p> Set the document base of the applet or bean.
     * We want to run the init() method of AppletPanel,
     * which starts the handler thread, when we have *both*:
     * - the window handle, and
     * - the codebase
     *
     * The browser can notify us of these in either order.
     * So the last one we get (window/stream) will also
     * fire the init method.  Otherwise, we can deadlock
     * with the browser if the other isn't ready yet.
     * </p>
     *
     * @param url Document base URL of the applet or bean.
     */
     public synchronized void setDocumentURL(String url)
     {
	 assert (panel != null);

         try 
	 {
             // Notify LiveConnect thread for scripting.
             notifyAll();

	     ((sun.plugin.AppletViewer)panel).setDocumentBase(url);

	     setDocBase = true;

             // Initialize an applet or bean
             initPlugin();

         } catch (Throwable e) {
	     Trace.printException(e);
         }
    }

    /** 
     * <p> Returns the embedded frame of the plugin object.
     * </p>
     *
     *  @returns Embedded frame of the plugin object.
     */
    private Frame getFrame()  
    {
	// This is used for printing
        return frame;
    }


    /**
     * <p> Set the focus on the embedded frame. This is a hack to make Swing
     * focus mechanism works inside Java Plug-in.
     * </p>
     */
    void setFocus()  {
	if (frame != null)
	    frame.synthesizeWindowActivation(true);
    }

    /** 
     * <p> Set the embedded frame size. This method is called only when the WIDTH
     * and HEIGHT in the EMBED tag are specified using relative dimension (%).
     * </p>
     *
     * @param width Width of the frame.
     * @param height Height of the frame.
     */
    void setFrameSize(int width, int height)  {

	final Object obj = getJavaObject();
	if (obj == null) {
	    this.width = width;
	    this.height = height;

            // Applet is not ready, but resize the embedded frame and
            // the AppletViewer (AppletPanel) so that graybox and its animation
            // can be sized relatively with the browser window.
            if (frame != null)
                frame.setSize(width, height);

            if (panel != null)
                panel.setBounds(0, 0, width, height);
	    
            return;
	}
	
	// Synchronization is important here to make sure the
	// width and the height are set are the same time.
	//
	if (width > 0 && height > 0)
	{
	    try 
	    {
		synchronized(this)  {			
		    // Reset the WIDTH and HEIGHT in the AppletContext.
		    setParameter("width", new Integer(width));
		    setParameter("height", new Integer(height));
		
			
	
		    // Resize the embedded frame.
		    if (frame != null)
			frame.setSize(width, height);

		    if (panel != null)
			{
			    panel.setBounds(0, 0, width, height);
							
			    Applet applet;
			    if (obj instanceof Applet)
				applet = (Applet)obj;
			    else {
				// This must be a bean.
				Component c = (Component) obj;
				c.setSize(width, height);
				applet = (Applet) c.getParent();
			    }

			    if (applet != null) {
				applet.resize(width, height);
				applet.setVisible(true);
			    }
			}
		}
	    }
	    catch(Throwable e) {
		Trace.printException(e);
	    }
	}
    }

	
    /**
     * <p> Obtain the reference of the applet or bean.
     * </p>
     *
     * @return Java object.
     */
    public Object getJavaObject()   
    {
	Object obj = null;

	if (panel != null)
	    obj = panel.getViewedObject();

	if (obj instanceof BeansApplet)
	{
	    BeansApplet applet = (BeansApplet) obj;
	    obj = applet.getBean();
	}

	return obj;
    }

	/**
	 * <p> Obtain the global reference of the native
	 *     Java object </p>
	 *
	 * @return integer
	 */
	private int nativeJavaObject = 0;
	
	public int getNativeJavaObject()
	{
	    if (nativeJavaObject == 0) {
		Object obj = getJavaObject();
		nativeJavaObject = convertToGlobalRef(obj);
	    }

	    return nativeJavaObject;
	}
		

    /** 
     * <p> Obtain the loading status of the applet or bean.
     * </p>
     *
     * @return loading status.
     */
    int getLoadingStatus()
    {
	if (panel != null)
	    return panel.getLoadingStatus();
	else
	    return AppletPanel.APPLET_ERROR;
    }


    /**
     * <p> Returns the value of a param from the param list.
     * </p>
     *
     * @param k Parameter key name.
     * @returns The value of the parameter.
     */
    public String getParameter(String k)
    {
	assert (panel != null);

	return panel.getParameter(k);
    }


    /**
     * <p> Set the parameter.
     * </p>
     *
     * @param name Parameter key name.
     * @param value Parameter value.
     */
    public void setParameter(String name, Object value)  
    {
	assert (panel != null);

	panel.setParameter(name, value);
    }

    /*
     * Set background/foreground and progress bar color for the applet viewer.
     */
    public void setBoxColors()
    {
        panel.setColorAndText();
    }

    /* Print the applet to the output stream, 'printOut'. Note - this
     * requires Mozilla 1.7 or later, or Mozilla FireFox 1.5 or later.
     * These browsers specify that the plugin generate EPSF, and only
     * support rendering plugin content to a single output page, so
     * content may be moved to the top of the next page and/or scaled
     * down to fit. This is a limitation we can't do anything about here.
     * "Plugin" is meant in the general browser plugin sense, not just Java.
     * Earlier versions of this code were different and supported only
     * Netscape Navigator 4.7x which did support page spanning output but
     * not without oddities. However that browser is now obsolete and
     * unsupported by Java Plugin.
     * No intermediate versions such as Navigator 6.X or earlier versions
     * of Mozilla or FireFox had useable support - completely unimplemented
     * in some cases, and unspecified and buggy in other cases.
     */
    public void doPrint(int x, int y, int w, int h, PrintStream printOut) {
        if (panel != null) {
            PSPrinterJob.PluginPrinter pp = new
                PSPrinterJob.PluginPrinter(panel, printOut, x, y, w, h);
            try {
                pp.printAll();
            } catch (Throwable t) {
                /* don't let applet errors hang/crash plugin */
                Trace.println("doPrint: printAll failed", TraceLevel.BASIC);
                Trace.printException(t);
            }
        }
    }

    private boolean evaluatingExp = false;
    private boolean returnedExp = false;
    private String returnJS = null;

    private Object syncObject = new Object();

    public String evalString(int instance, String jsexp) 
    {
	synchronized(syncObject)
	{
	    try {
		while(evaluatingExp)
		    syncObject.wait();
		evaluatingExp = true;
		returnedExp = false;
		Worker.sendJSRequest(instance, jsexp);
		while (!returnedExp)
		    syncObject.wait();
		evaluatingExp = false;
		String ret = returnJS;
		returnJS = null;
		syncObject.notifyAll();
		return ret;
	    } catch (Exception e) {
		evaluatingExp = false;
		returnJS =  null;
		syncObject.notifyAll();
		return null;
	    }
	}
    }
    
    public void setJSReply(String ret) {
	synchronized(syncObject)
	{
	    if (returnJS == null)
		returnJS= ret;
	    else 
		returnJS += ret;
	}
    }

    public void finishJSReply() {
	synchronized(syncObject)
	{
	    returnedExp = true;
	    syncObject.notifyAll();
	}
    }

    public void statusChanged(int status) {

        // "id" concept is employed inconsistently between Solaris NPI and
        // Windows NPI. For Solaris, the following range of ID is valid:
        // id >= 0. So conditional check as follow for Solaris NPI when
        // signaling status.
        if ( id >= 0 )          
        {
            Worker.notifyStatusChange(id, status);
            signal(status);
        }

    }

    private void destroyFrame()
    {
	final java.awt.Frame embeddedFrame = frame;

	try {
	    DeployAWTUtil.invokeLater(embeddedFrame, new Runnable() {
		public void run() {
		    // Hide the frame first. This is very important
		    // to hide the frame before calling dispose().
		    // Otherwise, some race condition with repaint()
		    // may be resulted.
		    //
		    embeddedFrame.setVisible(false);

		    // Close and remove frame.
		    embeddedFrame.setEnabled(false);

		    WindowEvent we = new WindowEvent(embeddedFrame, WindowEvent.WINDOW_CLOSING);
		    DeployAWTUtil.postEvent(embeddedFrame, we);
		}});
	} catch(Exception e) {
	}
    }

	private native int convertToGlobalRef(Object obj);
	private native void releaseGlobalRef(int gjobject);
}
