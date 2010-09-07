/*
 * @(#)WNetscapeEmbeddedFrame.java	1.25 03/12/19
 *
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.frame;


import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.EventQueue;
import sun.awt.SunToolkit;
import sun.plugin.viewer.WNetscapePluginObject;
import sun.plugin.services.PlatformService;
import com.sun.deploy.util.DeployAWTUtil;
import java.awt.Dialog;
import sun.awt.windows.WComponentPeer;
import java.lang.reflect.Field;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import com.sun.deploy.util.Trace;

/* The WNetscapeEmbeddedFrame holds the Panel, that holds the applet or JavaBeans.
 * The plugin is handed a native window object, embedded in the browser window
 * - so we extend EmbeddedFrame for this purpose.
 */
public class WNetscapeEmbeddedFrame extends sun.awt.windows.WEmbeddedFrame
{

    public WNetscapeEmbeddedFrame(long handle) 
    {
        super(handle);

        setLayout(new BorderLayout());
        setBackground(Color.white);
    }

    private WNetscapePluginObject obj = null;

    public void setJavaObject(WNetscapePluginObject obj)
    {
	this.obj = obj;
    }
        
    /*
     * Overwrite this method to notify browser window that modal dialog
     * is showing.
     */
    public void notifyModalBlocked(Dialog blocker, boolean blocked){
        super.notifyModalBlocked(blocker, blocked);
        
        // Should not call blocker.getPeer() on this thread.
        //long blockerHWnd = ((WComponentPeer)blocker.getPeer()).getHWnd();
                       
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
        enableModeless((int) getEmbedderHandle(), blocked, blockerHWnd);
    }
    
    
    public void activateEmbeddingTopLevel(){
        // Call native method to activate browser's window
        activateBrowserWindow((int) getEmbedderHandle());
    }
            
    /* Destroy the frame */
    public void destroy() {

        // Hide the frame first. This is very important
        // to hide the frame before calling dispose().
        // Otherwise, some race condition with repaint()
        // may be resulted.
        //
        setVisible(false);

        // Close and remove frame.
        setEnabled(false);

        //remove all components
        removeAll();

        //dispose the frame
        dispose();

        obj = null;
    }


    // native method to block browser's window while modal dialog is showing
    private native void enableModeless(int nativeHandler, boolean fEnable, 
            long blockerHandle);
    
    // native method to activate browser's window
    private native void activateBrowserWindow(int browserHandle);
            
}

