/*
 * @(#)WNetscapePluginObject.java	1.106 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.viewer;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.Panel;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.EventQueue;
import java.awt.event.WindowEvent;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;
import com.sun.deploy.util.DeployAWTUtil;
import com.sun.deploy.perf.DeployPerfUtil;
import sun.awt.AppContext;
import sun.awt.SunToolkit;
import sun.applet.AppletClassLoader;
import sun.applet.AppletEvent;
import sun.applet.AppletPanel;
import sun.plugin.BeansApplet;
import sun.plugin.AppletViewer;
import sun.plugin.BeansViewer;
import sun.plugin.ClassLoaderInfo;
import sun.plugin.util.NotifierObject;
import sun.plugin.viewer.frame.WNetscapeEmbeddedFrame;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.viewer.context.NetscapeAppletContext;
import sun.plugin.security.PluginClassLoader;
import sun.plugin.services.BrowserService;
import sun.plugin.services.PlatformService;
import com.sun.deploy.util.Trace;

/**
 * <p> WNetscapePluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class WNetscapePluginObject implements sun.plugin.AppletStatusListener
{
    /** 
     * <p> Embedded Frame object for the plugin.
     * </p>
     */
    private WNetscapeEmbeddedFrame frame = null;

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
      * <p> Parent window handle of the embedded frame.
      * </p>
      */
    private long handle = 0;

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
     * Flags indicating plugin status
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

    private boolean isLegacy = false;

    /**
     * <p> Create a WNetscapePluginObject.
     * </p>
     *
     * @param id Identifier of the Plugin instance.
     */
    WNetscapePluginObject(int id, boolean isBeans, String identifier)
    {
        this.id = id;
	this.identifier = identifier;

	// Obtain panel from LifeCycleManager
	this.panel = LifeCycleManager.getAppletPanel(identifier);

	if (isBeans == false)
	{
	    // Applet case
	    if (this.panel == null)
	    {
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
    private WNetscapeEmbeddedFrame createFrame(final long handle, final int width, final int height)  
    {
        DeployPerfUtil.put("START - Java   - ENV - create embedded browser frame (Mozilla:Windows)");

        //get ClassLoader, ThreadGroup and AppContext for the applet
        tmpLoader = panel.getAppletClassLoader();

	final WNetscapeEmbeddedFrame[] box = new WNetscapeEmbeddedFrame[1];

        final PlatformService ps = PlatformService.getService();
        final int frameEvent = ps.createEvent();

        Runnable task = new Runnable() {
            public void run() {
                try {
                    if (destroyed) return;

                    box[0] = new WNetscapeEmbeddedFrame(handle);
                    box[0].add(panel);

                    box[0].setVisible(true);
                    box[0].setEnabled(true);
                    box[0].setSize((int)width, (int)height);
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    ps.signalEvent(frameEvent);
                }
            }
        };

	if (tmpLoader != null) { 
	    DeployAWTUtil.invokeLater(tmpLoader.getAppContext(), task);
	} else {
	    //invoke in the main thread group
            DeployAWTUtil.invokeLater(AppContext.getAppContext(), task);
        }


	ps.waitEvent(handle, frameEvent, 5000);
        ps.deleteEvent(frameEvent);

        frame = box[0];

	frame.setJavaObject(this);

        DeployPerfUtil.put("END   - Java   - ENV - create embedded browser frame (Mozilla:Windows)");

	return frame;
    }

    private void destroyFrame() {
        final NotifierObject notifier = new NotifierObject();

        DeployAWTUtil.invokeLater(frame, new Runnable() {
            public void run() {
                try {
                    frame.destroy();
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    notifier.setNotified();
                }
            }
        });

        waitForNotification(notifier, 5000);
        if (!notifier.getNotified()) {
            throw new RuntimeException("Error while destroying embedded frame");
        }
    }

    /**
     * <p> Creates a new Frame according to the type of the Panel.
     * It is called by the Plugin when NPP_SetWindow is called.
     * </p>
     *
     * @param handle Window handle of the Plugin window.
     * @param width Width of the Plugin window (in pixel).
     * @param height Height of the Plugin window (in pixel).
     * @returns The embedded frame which hosts the applet/bean.
     */
    synchronized Frame setWindow(long handle, int width, int height)
    {
	
	// setWindow() is called whenever the plug-in is started,
	// stoped, and resized. When the plug-in first starts
	// up before Applet.start() is called, setWindow()
	// will be called with a valid plug-in window handle.
	// When the plug-in is destroyed, Applet.stop() will
	// be called, and followed by a setWindow() with a 
	// NULL plug-in window handle.
	if (width == 0) width = 1;
	if (height == 0) height = 1;

        // Frame has been created
        if (this.handle == handle)
            return frame;

        // A different handle is passed. Remove existing frame.
	if (frame != null)
	    frame.destroy();

        frame = null;
        this.handle = handle;

        // A different handle is passed, create new frame.
        if (handle != 0)
        {
	    try  {
		this.width = Integer.parseInt(getParameter("width"));
		this.height = Integer.parseInt(getParameter("height"));
	    } catch (NumberFormatException e) {
		setParameter("width", new Integer(width));
		setParameter("height", new Integer(height));
		this.width = width;
		this.height = height;
	    }
	    catch (Throwable e)
	    {
		// catch all ....
		Trace.printException(e);
	    }

            // create an embedded frame depends on the runtime type.
            frame = createFrame(handle, this.width, this.height);

            // Initialize an applet or bean
            initPlugin();
        }

        return frame;
    }

    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer panel;
	WNetscapePluginObject obj;

	Initer(ThreadGroup group, AppletViewer panel, WNetscapePluginObject obj) {
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

            //Mozilla browser is not affected by the bugs. 
            //Just to be consistent wit IE Plugin Object
            //see IExplorerPluginObject.java
            LifeCycleManager.initAppletPanel(panel);

	    synchronized(obj)
	    {
		obj.initialized = true;

		//6244718, plugin has been destroyed, don't start plugin
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

        // Start an applet or bean
	if (bInit == false && !destroyed)
	{
		bInit = true;

		// check life cycle
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


    /**
     * <p> This count is for remaining the number of pending request for starting 
     * the applet. It is used when the applet is not completely loaded but a 
     * start request is made.
     * </p>
     */
    private int startCount = -1;

    //flag indicating the applet is initialized and ready to start
    private volatile boolean initialized = false;

    //flag indicating whether initPlugin() is called
    private boolean bInit = false;

    private final Object syncStartStop = new Object(); 


    /** 
     * <p> Start an applet or bean.
     * </p>
     */
    void startPlugin()
    {
	assert (panel != null);

	if (initialized == true) {

            //startPlugin may be called on the Initer thread
            //stopPlugin may be called on the main thread
            //use syncStartStop to serialize two calls
            synchronized(syncStartStop) {
	        LifeCycleManager.startAppletPanel(panel);
            }
        } else {
            synchronized(this) {
	        if (startCount < 0) {
		    startCount = 0;
                }

	        startCount++;
            }
	}
    }

    /** 
     * <p> Stop an applet or bean.
     * </p>
     */
    void stopPlugin()
    {
	assert (panel != null);

	if (initialized == true) {
            synchronized(syncStartStop) {
                LifeCycleManager.stopAppletPanel(panel);
            }
        } else {
            synchronized(this) {
                startCount--;
            }
        }
    }

    void destroyPlugin() {
	destroyPlugin(0);
    }

    /**
     * <p> Remove the panel from the embedded frame and stop the applet/bean.
     * </p>
     */
    void destroyPlugin(long handle) 
    {
	assert (panel != null);

        final PlatformService ps = PlatformService.getService();
        final int destroyEvent = ps.createEvent();
        final NetscapeAppletContext pac =
                (NetscapeAppletContext) panel.getAppletContext();

        //create shutdown thread in the applet thread group
        Thread shutdownThread =
            new Thread(SunToolkit.targetToAppContext(frame).getThreadGroup(),
                                                      "applet-shutdown") {
                public void run() {
                    try {
                        synchronized (WNetscapePluginObject.this) {
                            destroyed = true;
                        }

                        if (!bInit) {
                            //destroy the frame
                            if (frame != null) {
                                destroyFrame();
                            }

                            return;
                        }

                        //if we reach here, plugin object is initialized
                        //need wait for loading completed before shutdown

                        panel.waitForLoadingDone(5000);

                        //set the flag to be false
                        isAppletStoppedOrDestroyed = false;

                        LifeCycleManager.destroyAppletPanel(identifier, panel);

                        if (panel.getAppletHandlerThread() != null
                                && getLoadingStatus() != AppletPanel.APPLET_ERROR) {
                            //wait up to 5 seconds for applet destroy
                            synchronized(syncDestroy) {
                                if (!isAppletStoppedOrDestroyed) {
                                    syncDestroy.wait(5000);
                                }
                            }
                        }
                        //reset applet context handle
                        pac.setAppletContextHandle(0);

                        //destroy the embedded frame
                        destroyFrame();
                    } catch (Exception e) {
                        e.printStackTrace();
                    } finally {
                        ps.signalEvent(destroyEvent);
                    }
                }
            };

        shutdownThread.start();
        // wait for the above applet shutdown thread while pump messages
        ps.waitEvent(handle, destroyEvent, 10000L);
        ps.deleteEvent(destroyEvent);

        //remove applet status listener
        panel.removeAppletStatusListener(null);

        // Release all JSObjects
        ((NetscapeAppletContext) pac).onClose();

        // When applet is stopped/destroyed, it is VERY important to
        // reset the applet context, so applet won't be able to
        // call methods in applet context to interface with the
        // browser if the document has gone away.
        //
        if (pac != null)
            pac.setAppletContextHandle(0);

        if (bInit) {
            LifeCycleManager.cleanupAppletPanel(panel);
        } else {
            panel.miniCleanup();
        }

	final AppletViewer viewer = panel;

        new Thread("finalCleanupThread") {
            public void run() {
                LifeCycleManager.releaseAppletPanel(viewer);
            }
        }.start();

        this.id = 0;
        panel = null;
        frame = null;
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
     * When browser sends window handle to use, we create
     * embedded frame. createFrame need get the codebase
     * first. If the codebase has not been set, createFrame
     * has to wait for the setDocumentURL(). So initPlugin() only
     * when the frame is created.
     * </p>
     *
     * @param url Document base URL of the applet or bean.
     */
     synchronized void setDocumentURL(String url)
     {
	 assert (panel != null);

         try 
	 {
             // Notify LiveConnect thread for scripting.
             notifyAll();

	     ((sun.plugin.AppletViewer)panel).setDocumentBase(url);

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
    void setFrameSize(final int width, final int height)  {

	// Synchronization is important here to make sure the
	// width and the height are set are the same time.
	//
        if ((width > 0) && (height > 0)) {
	    
	    // Reset the WIDTH and HEIGHT in the AppletContext.
	    //
            synchronized(this) {
	        setParameter("width", new Integer(width));
	        setParameter("height", new Integer(height));
            }

            if (frame == null) {
                return;
            }
            //use invoke later to avoid blocking the browser main thread

            DeployAWTUtil.invokeLater(frame, new Runnable() {
                public void run() {
	            // Resize the embedded frame.
		    frame.setSize(width, height);

	            // Resize the panel.
	            if (panel != null)  {
		        Panel p = panel;

		        p.setSize(width, height);	    

		        panel.setParameter("width", new Integer(width));
		        panel.setParameter("height", new Integer(height));

		        Object obj = panel.getViewedObject();
	
		        if (obj != null)
		        {	    
		            Applet applet;

		            if (obj instanceof Applet) {
			        applet = (Applet) obj;
		            }
		            else {
			        // This must be a bean.
			        Component c = (Component) obj;
			        c.setSize(width, height);
			        applet = (Applet) c.getParent();
    		            }

		        if (applet != null)
			    applet.resize(width, height);
		        }
	            }
                }
            }); 
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

    public void statusChanged(int status)
    {
        // "id" concept is employed inconsistently between Solaris and Windows
        // NPI. For Windows, the following range of ID are valid:
        // (id < 0) || (id > 0)
        // In the event zero id is sent from Java->Native, it's a no-op and so
        // protected. Therefore, no need to conditional check when signaling
        // status for Windows NPI.
        notifyStatusChange(id, status);
        signal(status);
    }

    private native void notifyStatusChange(int instance, int status);
}
