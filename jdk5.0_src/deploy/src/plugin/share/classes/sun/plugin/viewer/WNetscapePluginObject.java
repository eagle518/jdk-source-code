/*
 * @(#)WNetscapePluginObject.java	1.83 02/12/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.viewer;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Component;
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
import sun.awt.AppContext;
import sun.awt.SunToolkit;
import sun.applet.AppletListener;
import sun.applet.AppletEvent;
import sun.applet.AppletPanel;
import sun.plugin.BeansApplet;
import sun.plugin.AppletViewer;
import sun.plugin.BeansViewer;
import sun.plugin.util.Trace;
import sun.plugin.viewer.frame.WNetscapeEmbeddedFrame;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.viewer.context.NetscapeAppletContext;
import sun.plugin.services.BrowserService;
import com.sun.deploy.util.DeployAWTUtil;


/**
 * <p> WNetscapePluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class WNetscapePluginObject implements AppletListener,
    sun.plugin.AppletStatusListener
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
    private int handle = 0;

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

    private int	    waitEvent = 0;

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
		this.panel.addAppletListener(this);
	    }

	    // Obtain the applet context and reset the handle
	    PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();
	    
	    if (pac == null)
	    {
		BrowserService bs = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
		pac = bs.getAppletContext();
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
		pbc = bs.getBeansContext();
	    }

	    pbc.setAppletContextHandle(id);
	    this.panel.setAppletContext(pbc);
	}
    }

    private void setWaitEvent(int waitEvent) {
	assert(waitEvent != 0);
	assert(this.waitEvent == 0);
	this.waitEvent = waitEvent;
    }

    /**
     * <p> Create the embedded frame for an applet or beans depends on the 
     * runtime type.
     * </p>
     */
    private WNetscapeEmbeddedFrame createFrame(int handle)  
    {
	WNetscapeEmbeddedFrame frame = new WNetscapeEmbeddedFrame(handle);

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
	Applet a = panel.getApplet();

	if (a != null)
	{
	    AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
	}

	frame.setJavaObject(this);
	frame.setWaitingEvent(waitEvent);

	return frame;
    }

    /**
     * <p> Creates a new Frame according to the type of the Panel.
     * It is called by the Plugin when NPP_SetWindow is called.
     * </p>
     *
     * @param handle Window handle of the Plugin window.
     * @param waitHandle Event handle for synchronization
     * @param width Width of the Plugin window (in pixel).
     * @param height Height of the Plugin window (in pixel).
     * @returns The embedded frame which hosts the applet/bean.
     */
    synchronized Frame setWindow(int handle, int waitHandle, int width, int height)
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
            frame = createFrame(handle);

	    try
	    {	    
      		frame.setSize(width, height);

		// Make sure the frame is visible as soon as possible
		// This is very important for displaying the loading
		// message.
		frame.setVisible(true);
		frame.setEnabled(true);
	    }
	    catch(Throwable e)
	    {
		Trace.printException(e);
	    }

            // Initialize an applet or bean
            initPlugin();
        }

        return frame;
    }

    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer that;
	WNetscapePluginObject obj;

	Initer(AppletViewer that, WNetscapePluginObject obj) {
	    this.that = that;
	    this.obj = obj;
  	}
	/**
         * Main body of thread.  Call init on our PluginPanel.
	 */
	public void run() {

	    // This is important to use sync because "initPanel" must be 
	    // completed before the plugin can continue.
	    //
	    LifeCycleManager.initAppletPanel(that);

	    synchronized(obj)
	    {
		obj.initialized = true;

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
	    // Insert the applet/bean into the embedded frame.
	    // This is very important to make it visible as well.
	    //
	    try
	    {
		final Frame f = frame;

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
		Applet a = panel.getApplet();

		if (a != null)
		{
		    AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
		}
	
		// Use invoke later to avoid blocking and race condition
		// with browser window.
		//
		DeployAWTUtil.invokeLater(frame, new Runnable()
		{
		    public void run()
		    {
			f.add(panel);
			f.setVisible(true);
		    }
		});
	    }
	    catch(Exception e)
	    {
	    }

	    // Start an applet or bean
	    if (initialized == false)
	    {
		initialized = true;

		// check life cycle
		LifeCycleManager.checkLifeCycle(panel);

		// This is very important to call panel.initPanel() from a
		// different thread, because it may call back into the plugin
		// for auto config proxy during this call. Thus, it must be 
		// called from a secondary thread. This hack is mainly for JDK 1.2.
		//
		new Initer(panel, this).start();
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


    /** 
     * <p> Start an applet or bean.
     * </p>
     */
    synchronized void startPlugin()
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
    synchronized void stopPlugin()
    {
	assert (panel != null);

	if (initialized == true)
	    LifeCycleManager.stopAppletPanel(panel);
	else
	    startCount--;
    }

    void destroyPlugin() {
	destroyPlugin(0);
    }

    /**
     * <p> Remove the panel from the embedded frame and stop the applet/bean.
     * </p>
     */
     synchronized void destroyPlugin(int handle) 
     {
	assert (panel != null);

	PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();

	LifeCycleManager.destroyAppletPanel(identifier, panel);

	// When applet is stopped/destroyed, it is VERY important to
	// reset the applet context, so applet won't be able to
	// call methods in applet context to interface with the
	// browser if the document has gone away.
	//

	if (pac != null) {
	    pac.setAppletContextHandle(0);
	    ((NetscapeAppletContext)pac).onClose();
	}


	if (frame != null)
	    frame.destroy();

	this.id = 0;
		    
	panel = null;
	frame = null;
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
     synchronized void setDocumentURL(String url)
     {
	 assert (panel != null);

         try 
	 {
             // Notify LiveConnect thread for scripting.
             notifyAll();

	     ((sun.plugin.AppletViewer)panel).setDocumentBase(url);

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
	    frame.requestFocus(); 
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

	// Synchronization is important here to make sure the
	// width and the height are set are the same time.
	//
	synchronized(this)  {
          if ((width > 0) && (height > 0)) {
	    
	    // Reset the WIDTH and HEIGHT in the AppletContext.
	    //
	    setParameter("width", new Integer(width));
	    setParameter("height", new Integer(height));

	    // Resize the embedded frame.
	    if (frame != null)   {
		frame.setSize(width, height);
	    }

	    // Resize the panel.
	    if (panel != null)  {
		Panel p = panel;

		if (p != null)
		{
		    p.setSize(width, height);	    

		    panel.setParameter("width", new Integer(width));
		    panel.setParameter("height", new Integer(height));
		}

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


    public void appletStateChanged(AppletEvent evt) 
    {
	AppletPanel src = (AppletPanel)evt.getSource();

	switch (evt.getID()) {
	    case AppletPanel.APPLET_LOADING_COMPLETED: {

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
		if (frame != null)
		{
		    Applet a = src.getApplet(); // sun.applet.AppletPanel

		    if (a != null)
			AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
		    else
			AppletPanel.changeFrameAppContext(frame, AppContext.getAppContext());
    		}

		break;
	    }
	}
    }

    public void statusChanged(int status)
    {
		if (id != 0)
			notifyStatusChange(id, status);
    }

    private native void notifyStatusChange(int instance, int status);
}
