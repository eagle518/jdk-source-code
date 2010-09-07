/*
 * @(#)IExplorerPluginObject.java	1.40 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.viewer;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Component;
import java.awt.Panel;
import java.awt.EventQueue;
import java.awt.Frame;
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
import sun.plugin.security.PluginClassLoader;
import sun.plugin.util.NotifierObject;
import sun.plugin.viewer.IExplorerAppletStatusListener;
import sun.plugin.viewer.frame.IExplorerEmbeddedFrame;
import sun.plugin.viewer.context.IExplorerAppletContext;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.services.BrowserService;
import sun.plugin.services.PlatformService;
import sun.plugin.com.DispatchImpl;

/**
 * <p> IExplorerPluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class IExplorerPluginObject
{
    /** 
     * <p> Internet Explorer Applet Status Listener
     * </p>
     */
    protected IExplorerAppletStatusListener ieasl = null;

    /** 
     * <p> Embedded Frame object for the plugin.
     * </p>
     */
    protected IExplorerEmbeddedFrame frame = null;

    /** 
     * <p> Panel object for hosting the applet or bean.
     * </p>
     */
    protected AppletViewer panel = null;

    /** 
     * <p> ID of an applet or bean in Navigator.
     * </p>
     */
    protected int id = 0;

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
     * Identifier for applet instance.
     */
    private String identifier = null;

    /**
     * Flags indicating plugin status 
     */
    private boolean destroyed = false;
    private boolean started = false;

    private long hWnd = 0;
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
     * <p> Create a IExplorerPluginObject.
     * </p>
     *
     * @param id Identifier of the Plugin instance.
     */
    IExplorerPluginObject(int id, boolean isBeans, String identifier)
    {
        this.id = id;
	this.identifier = identifier;

	if (isBeans == false)
	{
	    // Obtain panel from LifeCycleManager
	    this.panel = LifeCycleManager.getAppletPanel(identifier);

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

    public IExplorerEmbeddedFrame createFrame(final long handle) {

	DeployPerfUtil.put("START - Java   - ENV - create embedded browser frame (IE)");
	hWnd = handle;

        //get ClassLoader, ThreadGroup and AppContext for the applet
	tmpLoader = panel.getAppletClassLoader();

	final PlatformService ps = PlatformService.getService();
	final int frameEvent = ps.createEvent();

	Runnable task = new Runnable() {
	    public void run() {
		try {
	            if (destroyed) return;

	            frame = new IExplorerEmbeddedFrame(handle, IExplorerPluginObject.this);
		    frame.setBean(panel);

		    frame.setVisible(true);
		    frame.setEnabled(true);
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

	ps.waitEvent(hWnd, frameEvent, 5000);
	ps.deleteEvent(frameEvent);

	ieasl = new IExplorerAppletStatusListener(id, this);
	ieasl.setEmbeddedFrame(frame);

	DeployPerfUtil.put("END   - Java   - ENV - create embedded browser frame (IE)");

        return frame;
    }

    protected void destroyFrame() {

	final IExplorerEmbeddedFrame ieef = frame;

	if (ieef != null) {
	    final NotifierObject notifier = new NotifierObject();

	    try {
		DeployAWTUtil.invokeLater(ieef, new Runnable() {
		    public void run() {
			try {
			    ieef.destroy();
			} catch (Exception e) {
			    e.printStackTrace();
			} finally {
			    notifier.setNotified();
			}
		    }
		});
	    } catch (Exception e) {
	    }

	    waitForNotification(notifier, 5000);
	    if (!notifier.getNotified()) {
		throw new RuntimeException("Error while destroying embedded frame");
	    }
	}
    }


    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer panel;
	IExplorerPluginObject obj;

	Initer(ThreadGroup group, AppletViewer panel , IExplorerPluginObject obj) {
            //make the initer run in the applet thread group
            super(group, "Plugin-Initer");

	    this.panel = panel;
	    this.obj = obj;
  	}

	/**
         * Main body of thread. It loads, inits and starts applet through AppletPanel 
	 */
	public void run() {
	    // This is important to use sync because "initPanel" must be 
	    // completed before the plugin can continue.
	    //
	    LifeCycleManager.loadAppletPanel(panel);

            if (obj.destroyed ) {
                return;
            }

            //factor out sendEvent(APPLET_INIT) code
            //if plugin object is in destroying, do not call
            //applet.init(). This is for IE DHTML bug 6534719
            //and 4751259. The control is activated and deactivated
            //immediately, then activate again. If we don't check,
            //applet.init() are called twice.

            LifeCycleManager.initAppletPanel(panel);

            //start applet. set started flag to true
            started = true;
	    LifeCycleManager.startAppletPanel(panel);
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

	if (bInit || destroyed) {
	    return;
	}

	//set bInit to true. From now on, destroyPlugin() need wait for
	//loading completion before it can shutdown plugin
	//if it doesn't wait, shutdown thread may race with Initer thread
	bInit = true;

	this.panel.addAppletStatusListener(ieasl);

	// life cycle check
	LifeCycleManager.checkLifeCycle(panel);

	// This is very important to call panel.initPanel() from a
	// different thread, because it may call back into the plugin
	// for auto config proxy during this call. Thus, it must be 
	// called from a secondary thread. This hack is mainly for JDK 1.2.
	//
        ThreadGroup group = (tmpLoader == null)? null : tmpLoader.getThreadGroup();
	new Initer(group, panel, this).start();
    }

    /*
     * <p>
     * The JavaBeans component or applet is being closed
     * </p>
     */
    public void destroyPlugin() 
    {
	assert (panel != null);

	final PlatformService ps = PlatformService.getService();
	final int destroyEvent = ps.createEvent();
	final IExplorerAppletContext pac = 
		(IExplorerAppletContext) panel.getAppletContext();

	//create shutdown thread in the applet thread group
	Thread shutdownThread = 
	    new Thread(SunToolkit.targetToAppContext(frame).getThreadGroup(), "applet-shutdown") {
                public void run() {
		    try {
			synchronized (IExplorerPluginObject.this) {
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

			//stop loading first
			panel.stopLoading();

			panel.waitForLoadingDone(5000);

                        //set the flag to be false
                        isAppletStoppedOrDestroyed = false;

                        if (started) {
			    LifeCycleManager.stopAppletPanel(panel);
                            started = false;
                        }

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

			// destroy the embedded frame
			destroyFrame();
			
			//clean up plugin object
			panel.removeAppletStatusListener(null);

			// Release all JSObjects
			((IExplorerAppletContext) pac).onClose();

		    } catch (Exception e) {
			e.printStackTrace();
		    } finally {
			ps.signalEvent(destroyEvent);
		    }
		}
	    };

	shutdownThread.start();

        // wait for the above applet shutdown thread while pump messages
        // wait time out is 10 seconds
        ps.waitEvent(hWnd, destroyEvent, 10000L);
        ps.deleteEvent(destroyEvent);

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
	frame = null;
	panel = null;

    }

    void signal(int status) {

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

    boolean bFrameReady = false;
    boolean bContainerReady = false;
    volatile boolean bInit = false;


    /**
     * This is a workaround for supporting <applet> tag in
     * Internet Explorer. When <applet> tag is used, 
     * the ActiveX control may be called differently from
     * <object> tag. Thus, to ensure applet is properly 
     * supported, applet should be initialized iff
     *
     * - The frame is ready and its size is > (0,0)
     * - The container is ready
     */
    public void mayInit()
    {
	if (bFrameReady == true && bContainerReady == true && bInit == false)
	{
	    initPlugin();	    
	}
    }

    /**
     * Call if the container is ready
     */
    public void containerReady()
    {
	bContainerReady = true;
	mayInit();
    }

    /**
     * Call if the frame is ready
     */
    public void frameReady()
    {
	bFrameReady = true;
	mayInit();
    }

    
    /**
     * <p> Pre-refresh the plugin object
     * </p>
     */
    public void preRefresh()
    {
	if (panel != null)
	    panel.preRefresh();
    }

    /** 
     * <p> Returns the embedded frame of the plugin object.
     * </p>
     *
     *  @returns Embedded frame of the plugin object.
     */
    public sun.plugin.AppletViewer getPanel()  
    {
        return panel;
    }

    /** 
     * <p> Returns the embedded frame of the plugin object.
     * </p>
     *
     *  @returns Embedded frame of the plugin object.
     */
    protected Frame getFrame()  
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

	return obj;
    }


    public Object getDispatchObject()   
    {
	return new DispatchImpl(getJavaObject(), id);
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
     
}
