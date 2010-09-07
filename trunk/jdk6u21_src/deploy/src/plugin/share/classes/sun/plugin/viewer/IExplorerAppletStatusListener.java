/*
 * @(#)IExplorerAppletStatusListener.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

import sun.applet.AppletPanel;
import sun.plugin.viewer.frame.IExplorerEmbeddedFrame;
import com.sun.deploy.util.DeployAWTUtil;

/**
 * IExplorerAppletStatusListener is a concrete class for listening applet
 * status changes.
 */
public class IExplorerAppletStatusListener implements sun.plugin.AppletStatusListener
{    
    private int handle = 0;
    private IExplorerPluginObject pluginObject;
    protected IExplorerEmbeddedFrame frame = null;

    IExplorerAppletStatusListener(int handle, IExplorerPluginObject obj)
    {
	this.handle = handle;
	this.pluginObject = obj;
    }
    
    /**
     * Notify applet status change
     */
    public void statusChanged(int status)
    {
	if(status == sun.applet.AppletPanel.APPLET_START){
	    final IExplorerEmbeddedFrame f = (IExplorerEmbeddedFrame)frame;
	    try {
		DeployAWTUtil.invokeLater(f, new Runnable() {
		    public void run() {
			try {
			    f.synthesizeWindowActivation(true);
			} catch (NullPointerException e) {
			    //the frame may have been disposed
			}
		    }
		});
	    } catch(Exception exc) {
		exc.printStackTrace();
	    }
	}
	notifyStatusChange(handle, status);

	//Notify pluginOjbect about the status change
	//especially APPLET_STOP and APPLET_DESTROY
	pluginObject.signal(status);
    }

    // set the Internet Explorer embedded frame
    public void setEmbeddedFrame(IExplorerEmbeddedFrame ieef)  
    {
	this.frame = ieef;
    }

    // Native code to notify the changes
    private native void notifyStatusChange(int handle, int status);
}   


