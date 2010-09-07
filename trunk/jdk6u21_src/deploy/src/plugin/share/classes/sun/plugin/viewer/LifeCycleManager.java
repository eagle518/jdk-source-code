/*
 * @(#)LifeCycleManager.java	1.25 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Set;
import sun.applet.AppletPanel;
import sun.plugin.AppletViewer;
import sun.plugin.services.BrowserService;
import java.awt.Frame;
import com.sun.deploy.util.Trace;

/*
 * Life cycle manager for managing all the stopped applets
 */
public class LifeCycleManager
{
	// Browser hook installed
	// @since 1.4.1
	private static Object   browserListenerLock = new Object(); 
	private static boolean  browserListenerInstalled = false; 	
		    
    /**
     * Return an applet panel that matches the identifier
     */
    public static AppletViewer getAppletPanel(String identifier)    
    {
	AppletViewer viewer = null;

	synchronized(appletPanels)
	{
	    viewer = (AppletViewer) appletPanels.remove(identifier);
	}

	if (viewer != null)
	    Trace.msgPrintln("lifecycle.applet.found");

	return viewer;
    }

	/**
	 * check applet life cycle type, and install browser event listener
	 * if needed
	 * @since 1.4.1
	 */
	 public static void checkLifeCycle(AppletViewer panel) {
		if(panel.isLegacyLifeCycle())
			installBrowserEventListener();
	 }

	/**
	 * install browser event listener
	 * @since 1.4.1
	 */
	private static void installBrowserEventListener() {
		// return without acquiring lock
		if(browserListenerInstalled)
			return;

		synchronized(browserListenerLock) {
			if(!browserListenerInstalled) {
				BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
				browserListenerInstalled = service.installBrowserEventListener();
			}
		}
	}
    /* 
     * Add a appletPanel in this object cache
     * 
     * @param appletPanel the appletPanel to add
     */
    private static void add(String identifier, AppletViewer appletPanel) 
    {
		// List to store to-be-destroyed applet panel
		ArrayList list = new ArrayList();

		synchronized(appletPanels)
		{
			// Put applets into another list if the number of
			// running applets are too large
			//
			int numToPrune = appletPanels.size() - Integer.getInteger("javaplugin.lifecycle.cachesize", 1).intValue();

			if (numToPrune < 0)
			numToPrune = 0;

			Set keySet = appletPanels.keySet();
			Iterator iter = keySet.iterator();
			
			for (int i=0; i < numToPrune && iter.hasNext(); i++)
			{
			String k = (String) iter.next();

			// Put the appletPanel temporatory in a list		
			list.add(appletPanels.remove(k));
			}

			// Store new instance
			appletPanels.put(identifier, appletPanel);
		}

		if (list.size() > 0)
			Trace.msgPrintln("lifecycle.applet.cachefull");

		// Iterate the list and destroy each applet in the list.
		// Notice that it is very important to call destroy 
		// OUTSIDE the synchronization block
		//	
		for (Iterator iter = list.iterator(); iter.hasNext(); )
		{
			// Destroy the applet
			AppletViewer panel = (AppletViewer) iter.next();
			panel.appletDestroy();
			}

		list.clear();	    
    }


    /**
     * Determine the proper identifier given applet's parameters.
     */
    public static String getIdentifier(String[] k, String[] v)
    {
	// Create special string
	StringBuffer buffer = new StringBuffer();

	for (int i = 0; i < k.length; i++)
	{
	    if (k[i] != null)
	    {
		buffer.append("<NAME=");
		buffer.append(k[i]);
		buffer.append(" VALUE=");
		buffer.append(v[i]);
		buffer.append(">");
	    }
	}

	return buffer.toString();
    }

    /**
     * Load an applet through applet panel.
     *
     * @param panel Applet panel
     */
    public static void loadAppletPanel(AppletViewer panel)
    {
	if (panel.getLoadingStatus() == sun.applet.AppletPanel.APPLET_STOP ||
	    panel.getLoadingStatus() == sun.applet.AppletPanel.APPLET_INIT)
	{
            panel.notifyLoadingDone();
	} 
	else
	{
	    panel.appletInit();
	}
    }

    /**
     * Initialize applet.
     *
     * @param panel Applet panel
     */
    public static void initAppletPanel(AppletViewer panel)
    {
        if (panel.getLoadingStatus() == sun.applet.AppletPanel.APPLET_STOP ||
            panel.getLoadingStatus() == sun.applet.AppletPanel.APPLET_INIT)
        {
        }
        else
        {
            panel.sendAppletInit();
        }
    }


    /**
     * Start an applet through applet panel.
     *
     * @param panel Applet panel
     */
    public static void startAppletPanel(AppletViewer panel)
    {
	panel.appletStart();	    
    }

    /**
     * Stop an applet through applet panel.
     *
     * @param panel Applet panel
     */
    public static void stopAppletPanel(AppletViewer panel)
    {
		panel.appletStop();	    
    }

    /**
     * Destroy an applet through applet panel.
     *
     * @param panel Applet panel
     */
    public static void destroyAppletPanel(String identifier, AppletViewer panel)
    {
		// Check applet lifecycle
		if (panel.isLegacyLifeCycle())
		{
			Trace.msgPrintln("lifecycle.applet.support");
			// Legacy lifecycle
			// Store the panel into Lifecycle manager
			LifeCycleManager.add(identifier, panel);
		}
		else
		{
			// New lifecycle
			// Call applet.destroy() to clean up the applet
    		panel.appletDestroy();
		}
    }

    /**
     * Cleanup applet panel if it is not legacy lifecycle
     *
     * @param panel Applet panel
     */
    public static void cleanupAppletPanel(AppletViewer panel)
    {
        if (panel.isLegacyLifeCycle()) {
            //do nothing
        } else {
            panel.cleanup();
        }
    }

    /**
     * Dispose applet threadgroup and appcontext
     *
     * @param panel Applet panel
     */
    public static void releaseAppletPanel(AppletViewer panel)
    {
        if (panel.isLegacyLifeCycle()) {
            //do nothing
        } else {
            panel.release();
        }
    }

	/**
	 * Destroy all cached legacy life cycle applet panels. This will invoke
	 * applet's destroy method. Only happens during browser shutdown or
	 * browser profile switch (Netscape 6)
	 * 
	 * @since 1.4.1
	 */
	public static void destroyCachedAppletPanels() {
		AppletViewer panel;
		LinkedHashMap panels;
		synchronized(appletPanels) {
			panels = (LinkedHashMap)appletPanels.clone();
		}

		Iterator i = panels.values().iterator();
		// Cached applets no longger have embedded frame as their parent.
        // To avoid null pointer exception, here is the trick.
		Frame f = new Frame();
		f.toFront();		
		while(i.hasNext()) {
			panel = (AppletViewer)i.next();
			// wait until applet's destroy method fully executed
			f.add(panel);
 			panel.appletDestroy();
			panel.joinAppletThread();
			panel.cleanup();
			panel.release();
			f.remove(panel);
		}
		f.dispose();
	}

    // Use LinkedHashMap so it has predictable iteration order. It 
    // maintains a doubly-linked list running through all of its 
    // entries. This linked list defines the iteration ordering, 
    // which is normally the order in which keys were inserted into 
    // the map (insertion-order). 
    //
    private static LinkedHashMap appletPanels = new LinkedHashMap();
}   

