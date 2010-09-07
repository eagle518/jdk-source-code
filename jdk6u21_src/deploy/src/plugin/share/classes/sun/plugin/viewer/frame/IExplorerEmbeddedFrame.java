/*
 * @(#)IExplorerEmbeddedFrame.java	1.78 02/12/17
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.viewer.frame;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Toolkit;
import java.awt.event.InvocationEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.applet.Applet;
import java.beans.Beans;
import java.lang.reflect.InvocationTargetException;
import sun.awt.SunToolkit;
import sun.plugin.AppletViewer;
import sun.plugin.util.NotifierObject;
import sun.plugin.viewer.IExplorerPluginObject;
import sun.plugin.services.PlatformService;
import com.sun.deploy.util.DeployAWTUtil;
import java.awt.Dialog;
import java.awt.Frame;
import sun.awt.windows.WComponentPeer;
import java.lang.reflect.Field;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import com.sun.deploy.util.Trace;

/*
 * This is the java window used to inplace edit a JavaBeans component
 * An IExplorerEmbeddedFrame does not have decorations, always live within a 
 * container window (specified by the container application).
 * The lifetime of an IExplorerEmbeddedFrame is decided by the OLE container
 * Each OleEmbeddedWindow will itself embed a unique JavaBeans component
 *
 * @version 1.2.2
 * @date 12/15/98
 * @author Jerome Dochez
 */
public class IExplorerEmbeddedFrame extends sun.awt.windows.WEmbeddedFrame
{
    private IExplorerPluginObject pluginObj = null;

    /*
     * Build a new in place window and set its parent to the window 
     * handle specified. 
     *
     * @param nativeHandler native window implementation
     * @param hWnd Native Window handle of the parent window
     */
    public IExplorerEmbeddedFrame(long hWnd, IExplorerPluginObject obj) 
    {
        super(hWnd);

	pluginObj = obj;                

	// Use borderlayout, so the panel/applet/beans will always 
	// be the same size as the frame.
	setLayout(new BorderLayout());
    }

    /*
     * Specify which JavaBean component should be painted inside this
     * embedded frame window. 
     * 
     * @param bean the JavaBeans reference
     */
    private void setComponent(Component c) 
    {
	if (c!=null) {
	    // get the component original size
	    Dimension d = c.getSize();

	    add(c);

	    // Set the size to the original value
	    if (d.width!=0 && d.height!=0)
		setSize(d);
    
	    // Now that we have a window, start if this is a applet !
	    if (Beans.isInstanceOf(c, Applet.class)) {
		Applet apl = (Applet) Beans.getInstanceOf(c, Applet.class);
		apl.start();
	    }
	}
    }

    /*
     * Set the JavaBeans component as the embedded object.If the JavaBeans is
     * a visible bean, it supports java.awt.Component by direct inheritance or
     * by aggregation. If the bean is an invisible bean, then this method 
     * does nothing.
     * 
     * @param c Component
     */
    public void setBean(final Object component) {
                        try {
                            c =(java.awt.Component) Beans.getInstanceOf(component,
                                                                        Component.class);
                        } catch (ClassCastException e) {
                            return;
                        }

                        if (c != null)
                            setComponent(c);
    }


    /*
     * <p>
     * destroy frame. should be invoked on event diaptcher thread
     * </p>
     */
    public void destroy() {

        // Hide the frame first. 
        setVisible(false);

        // Close and remove frame.
        setEnabled(false);

	//remove all components
	removeAll();

	//dispose the frame
	dispose();

        pluginObj = null;
        c = null;
    }


    /*
     * Set the size of the embedded window, if the embedded window 
     * currently embed a component, resets its size as well.
     * 
     * @param width of the embedded frame
     * @param height of the embedded frame
     */
    public void setFrameSize(final int width, final int height) 
    {
	// If width != 0 && height !=0, then the appletviewer
	// may begin to run.
	//
	final int theWidth = (width == 0) ? 1 : width;
	final int theHeight = (height == 0) ? 1 : height;

	try
	{
	    // Set parameter in main thread
	    synchronized(this)
	    {
		if (c instanceof AppletViewer) {
		    AppletViewer panel = (AppletViewer) c;
		    panel.setParameter("width", Integer.toString(theWidth));
		    panel.setParameter("height", Integer.toString(theHeight));
		}
	    }

	    // Use invoke later to avoid blocking and race condition
	    // with browser window.
	    //
	    DeployAWTUtil.invokeLater(this, new Runnable()
	    {
		public void run()
		{
		    // Resize embedded frame
		    setSize(theWidth, theHeight);

		    // Resize AppletViewer/applet/beans
		    Component tmp = c;
		    if (tmp !=null) 
		    {
			tmp.setBounds(0, 0, theWidth, theHeight);

			if (tmp instanceof AppletViewer)
			{
			    AppletViewer panel = (AppletViewer) tmp;
			    panel.appletResize(theWidth, theHeight);

			    Applet applet = panel.getApplet();

			    if (applet != null)
				applet.resize(theWidth, theHeight);
			}
		    }
		}
	    });

	    // Notify the pluginObj that the frame is ready 
	    // in the main thread
	    pluginObj.frameReady();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}

    }

    protected boolean traverseOut(boolean direction) {
	transferFocus((int)getEmbedderHandle(), direction);
	return true;
    }

    /*
     * Overwrite this method to notify browser window that modal dialog
     * is showing.
     */
    public void notifyModalBlocked(Dialog blocker, boolean blocked){
       // super.notifyModalBlocked(blocker, blocked);
        
        // Should not call blocker.getPeer() on this thread.
        // long blockerHWnd = ((WComponentPeer)blocker.getPeer()).getHWnd();
        
        // ...hense all the dance.
        Field peerField = null;
        long blockerHWnd = 0;

        try {
            peerField = (Field)AccessController.doPrivileged(
                    new PrivilegedExceptionAction() {
                public Object run() throws Exception {
                    Field f = (Component.class).getDeclaredField("peer");
                    f.setAccessible(true);
                    return f;
                }
            });    
            
            WComponentPeer blockerPeer = 
                    (WComponentPeer)(peerField.get(blocker));
            blockerHWnd = blockerPeer.getHWnd();
            
        } catch (Exception ex) {
            Trace.printException(ex);
        }

        // If blockerHWnd ends up being 0 the browser 
        // window will not get blocked.
        
        // Call native method to block/unblock the browser window
        enableModeless((int)getEmbedderHandle(), blocked, blockerHWnd);
    }
    
    public void activateEmbeddingTopLevel(){
        // Call native method to activate browser's window
        activateBrowserWindow((int)getEmbedderHandle());
    }
    
        
    // native code to give focus to the next/previous ActiveX control
    private native void transferFocus(int handle, boolean direction);
    
    // native method to activate browser's window
    private native void activateBrowserWindow(int browserHandle);

    // native method to block browser's window while modal dialog is showing
    public native void enableModeless(int nativeHandler, boolean fEnable, 
            long blockerHandle);    

    /* 
     * reference to the JavaBeans component
     */
    private Component c;
}



