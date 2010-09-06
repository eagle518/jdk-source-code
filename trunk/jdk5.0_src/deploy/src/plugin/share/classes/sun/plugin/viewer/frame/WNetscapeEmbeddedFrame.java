/*
 * @(#)WNetscapeEmbeddedFrame.java	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.frame;


import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import sun.awt.SunToolkit;
import sun.plugin.util.Trace;
import sun.plugin.viewer.WNetscapePluginObject;
import sun.plugin.services.PlatformService;
import com.sun.deploy.util.DeployAWTUtil;

/* The WNetscapeEmbeddedFrame holds the Panel, that holds the applet or JavaBeans.
 * The plugin is handed a native window object, embedded in the browser window
 * - so we extend EmbeddedFrame for this purpose.
 */
public class WNetscapeEmbeddedFrame extends sun.awt.windows.WEmbeddedFrame
				  implements WindowListener, 
					     sun.awt.windows.ModalityListener
{
    private int win_handle;

    public WNetscapeEmbeddedFrame(int handle) 
    {
        super((long)handle);
        win_handle = handle;

        setLayout(new BorderLayout());
        setBackground(Color.white);
	addWindowListener(this);

	sun.awt.windows.WToolkit.getWToolkit().addModalityListener(this);
	Trace.msgPrintln("modality.register");
    }

    private WNetscapePluginObject obj = null;

    public void setJavaObject(WNetscapePluginObject obj)
    {
	this.obj = obj;
    }

    /**
     * <p> Set the focus on the embedded frame. This is a hack to make Swing
     * focus mechanism works inside Java Plug-in.
     * </p>
     */
    public void requestFocus()  
    {
	WindowEvent evt = new WindowEvent(this, WindowEvent.WINDOW_ACTIVATED);
	DeployAWTUtil.postEvent(this, evt);

	if (obj != null)
	{
	    Component c = (Component) obj.getJavaObject();

	    if (c != null)   {  
    		c.requestFocus();
	    }
	}
	else
	{
	    super.requestFocus();
	}
    }

    public void windowActivated(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowClosing(WindowEvent e) 
    { 
	try 
	{
	    // Remove window listener
	    removeWindowListener(this);

	    // Unregister
	    sun.awt.windows.WToolkit.getWToolkit().removeModalityListener(this);
	    Trace.msgPrintln("modality.unregister");

	    // Remove all components
	    removeAll();

	    // Reset object
	    obj = null;

	    // Dispose the frame
	    dispose(); 
	}
	catch(Throwable ex)
	{
	     Trace.printException(ex);
	}
	finally 
	{
	    // Notify frame dispose
	    if (handle != 0)
		PlatformService.getService().signalEvent(handle);
	}

    }
    public void windowDeactivated(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}

    public void waitEvent(int handle) {
	PlatformService.getService().waitEvent(handle);
    }

    private int handle;
    
    public void setWaitingEvent(int handle) {
	this.handle = handle;
    }

    /** 
     * Called by AWT when it enters a new level of modality
     */
    public void modalityPushed(sun.awt.windows.ModalityEvent evt)
    {
	Trace.msgPrintln("modality.pushed");
	enableModeless(win_handle, false);
    }


    /**
     * Called by AWT when it exits a level of modality
     */
    public void modalityPopped(sun.awt.windows.ModalityEvent evt)
    {
	Trace.msgPrintln("modality.popped");
	enableModeless(win_handle, true);
    }

    /* Destroy the frame */
    public void destroy() {
        final java.awt.Frame frame = this;

	try {
	    DeployAWTUtil.invokeLater(this, new Runnable() {
		public void run() {
		    // Hide the frame first. This is very important
		    // to hide the frame before calling dispose().
		    // Otherwise, some race condition with repaint()
		    // may be resulted.
		    //
		    frame.setVisible(false);

		    // Close and remove frame.
		    frame.setEnabled(false);

		    WindowEvent we = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
		    DeployAWTUtil.postEvent(frame, we);
		}});
	} catch(Exception e) {
	}


	assert(handle != 0);
	waitEvent(handle);
    }


    private native void enableModeless(int handle, boolean fEnable);
}

