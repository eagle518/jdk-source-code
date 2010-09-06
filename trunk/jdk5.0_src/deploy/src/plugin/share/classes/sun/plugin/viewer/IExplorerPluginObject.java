/*
 * @(#)IExplorerPluginObject.java	1.24 02/12/17
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
import sun.plugin.viewer.frame.IExplorerEmbeddedFrame;
import sun.plugin.viewer.context.IExplorerAppletContext;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.services.BrowserService;
import sun.plugin.com.DispatchImpl;

/**
 * <p> IExplorerPluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class IExplorerPluginObject implements AppletListener
{
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
    protected int id = -1;

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
	}
    }

    /**
     * <p> Create the embedded frame for an applet or beans depends on the 
     * runtime type.
     * </p>
     */
    public IExplorerEmbeddedFrame createFrame(int handle)  
    {
	frame = new IExplorerEmbeddedFrame(handle, this);

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

	frame.setBean(panel);

	return frame;
    }

    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer that;
	IExplorerPluginObject obj;

	Initer(AppletViewer that, IExplorerPluginObject obj) {
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
	    LifeCycleManager.startAppletPanel(that);
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

	this.panel.addAppletStatusListener(new IExplorerAppletStatusListener(id));

	// life cycle check
	LifeCycleManager.checkLifeCycle(panel);

	// This is very important to call panel.initPanel() from a
	// different thread, because it may call back into the plugin
	// for auto config proxy during this call. Thus, it must be 
	// called from a secondary thread. This hack is mainly for JDK 1.2.
	//
	new Initer(panel, this).start();
    }

    /*
     * <p>
     * The JavaBeans component or applet is being closed
     * </p>
     *
     * @param timeOut max time to wait for the applet to die
     */
    public void destroyPlugin() 
    {
	assert (panel != null);

	// Shutdown applet only if it has been initialized.
	if (bInit)
	{
	    PluginAppletContext pac = (PluginAppletContext) panel.getAppletContext();

	    LifeCycleManager.stopAppletPanel(panel);
	    LifeCycleManager.destroyAppletPanel(identifier, panel);

	    panel.removeAppletStatusListener(null);

	    // Release all JSObjects
	    ((IExplorerAppletContext) pac).onClose();


	    // When applet is stopped/destroyed, it is VERY important to
	    // reset the applet context, so applet won't be able to
	    // call methods in applet context to interface with the
	    // browser if the document has gone away.
	    //

	    if (pac != null)
		pac.setAppletContextHandle(0);
	}
    }


    boolean bFrameReady = false;
    boolean bContainerReady = false;
    boolean bInit = false;


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
	    bInit = true;

	    initPlugin();	    
	}
    }


    /**
     * Call if the container is ready
     */
    public void containerReady()
    {
	bContainerReady = true;

	// Initialize the
	mayInit();
    }

    /**
     * Call if the frame is ready
     */
    public void frameReady()
    {
	bFrameReady = true;

	// Initialize the
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
     * <p> Set the embedded frame size. This method is called only when the WIDTH
     * and HEIGHT in the EMBED tag are specified using relative dimension (%).
     * </p>
     *
     * @param width Width of the frame.
     * @param height Height of the frame.
     */
    void setFrameSize(int width, int height)  
    {
	if (frame != null)
	    frame.setFrameSize(width, height);
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
}
