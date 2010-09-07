/*
 * @(#)PluginSysUtil.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.EventQueue;
import java.awt.event.InvocationEvent;
import java.awt.Rectangle;
import java.awt.Point;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import java.awt.Toolkit;
import java.awt.Frame;
import java.security.AccessController;
import java.lang.reflect.InvocationTargetException; 
import javax.swing.JDialog;
import sun.awt.AppContext;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeployUIManager;


/**
 * <p>
 * This utility class is for plugin system thread group.
 * Since 1.4.2, we add a new thread group under main thread group, and this
 * thread group should be only used by plugin *system* code, such as Java
 * console window, exception dialog and download progress dialog, etc.
 * The newly created thread group will provide the separation of applets and
 * plugin system components, and prevent applets access these components.
 * @since 1.4.2
 * @author zhengyu.gu@sun.com
 */

public final class PluginSysUtil extends DeploySysRun 
{
    private static EventQueue	pluginEventQueue = null;
    private static ThreadGroup	pluginThreadGroup = null;
    private static ClassLoader	pluginSysClassLoader = null;
    /**
     * <p>
     * @return plugin system thread group
     * </p>
     */
    public synchronized static ThreadGroup getPluginThreadGroup() {
	// This method has to be called first by main thread 
	if(pluginThreadGroup == null) {
	    pluginSysClassLoader = 
		Thread.currentThread().getContextClassLoader();

	    pluginThreadGroup = new ThreadGroup(
		Thread.currentThread().getThreadGroup(), 
		"Plugin Thread Group");

	    createAppContext(pluginThreadGroup);

	    try  {
		Thread t = new Thread(pluginThreadGroup,
  		    new Runnable() {
			public void run() {
			    DeployUIManager.setLookAndFeel();
			}
		    });
		
		t.start();
		
		// It is critical to wait until the thread exits, so 
		// the look and feel of the AppContext would be set
		// properly before it is used for displaying dialogs.
		t.join();		
	    } 
	    catch(InterruptedException e)  {
		// no-op
	    }
	}

	return pluginThreadGroup;
    }

    /**
     * <p>
     *	To have runner running on plugin system event dispatch thread
     *</p>
     */
    public static void invokeAndWait(Runnable runner)
	throws InterruptedException, InvocationTargetException {

        if (EventQueue.isDispatchThread()) {
            throw new Error("Cannot call invokeAndWait from the event dispatcher thread");
        }

	class AWTInvocationLock {}
	Object lock = new AWTInvocationLock();

	InvocationEvent event = 
	    new InvocationEvent(Toolkit.getDefaultToolkit(), runner, lock, true);

	synchronized (lock) {
	    pluginEventQueue.postEvent(event);
	    lock.wait();
	}

	Exception eventException = event.getException();
	if (eventException != null) {
	    throw new InvocationTargetException(eventException);
	}
    }

    /**
     * <p>
     *	To have runner running on plugin system event dispatch thread
     *</p>
     */
    public static void invokeLater(Runnable runner) {
	pluginEventQueue.postEvent(new InvocationEvent(Toolkit.getDefaultToolkit(), runner));
    }

    /**
     * <p>
     * Create a thread inside plugin system thread group
     * </p>
     */
    public static Thread createPluginSysThread(Runnable runner) {
	Thread t = new Thread(pluginThreadGroup, runner);
	t.setContextClassLoader(pluginSysClassLoader);
	return t;
    }

    /**
     * <p>
     * Create a thread inside plugin system thread group
     * </p>
     */
    public static Thread createPluginSysThread(Runnable runner, String name) {
	Thread t = new Thread(pluginThreadGroup, runner, name);
	t.setContextClassLoader(pluginSysClassLoader);
	return t;
    }

    /**
     * Dummy dialog to workaround AWT issue
     */
    private static class DummyDialog extends JDialog {
	private ThreadGroup _unsecureGroup;

	DummyDialog(Frame owner, boolean isModal) {
	    super(owner, isModal);
	    _unsecureGroup = Thread.currentThread().getThreadGroup();
	}

	public void secureHide() {
            (new Thread(_unsecureGroup, new Runnable() {
                    public void run() { 
			DummyDialog.this.setVisible(false); 
		    }
                } )
            ).start();
	}
    }    


    public Object delegate(DeploySysAction action) throws Exception
    {
	return execute(action);
    }

    /**
     * <p>
     * Execute the <code>DeploySysAction</code> in plugin system thread group
     * </p>
     */    
    public static Object execute(DeploySysAction action) 
	throws Exception {

	// If the calling thread already in plugin thread group, then just call it	
	if(Thread.currentThread().getThreadGroup().equals(pluginThreadGroup)) {
	    return action.execute();
	}

	final SysExecutionThread t = new SysExecutionThread(action);
	t.setContextClassLoader(pluginSysClassLoader);

	// If the caller is on dispatch thread, it must be from other thread group.
	// In the case, to workaround AWT focus issue, we have to use dummy dialog
	// to have current event dispatch thread continue pumping event.
	if (EventQueue.isDispatchThread()) {
	    synchronized(t.syncObject) {
		final DummyDialog dummy = new DummyDialog(null, true);
		t.theDummy = dummy;
		dummy.addWindowListener(new WindowAdapter() {
		    public void windowOpened(WindowEvent e) {
			t.start();
		    }	    

		    public void windowClosing(WindowEvent e) {
			dummy.setVisible(false);
		    }
		});
		
		Rectangle rect = new Rectangle(new Point(0, 0),
		Toolkit.getDefaultToolkit().getScreenSize());
		if(!isOnWindows()) {
		    // make it behind real dialog
		    dummy.setLocation(rect.x + rect.width/2 - 50,
					 rect.y + rect.height/2);
		} else {
		    // make the dummy dialog off screen
    		    dummy.setLocation(-100, -100);
		}
		dummy.setResizable(false);
		dummy.toBack();
		dummy.setVisible(true);

		try {
		    t.syncObject.wait();
		} catch(InterruptedException e) {
		} finally {
		    dummy.setVisible(false);
		}
	    }
	}
	else {
	    t.start();
	    try {
		t.join();
	    } catch(InterruptedException e) {
	    }

	}
	if(t.exception != null)
	    throw t.exception;

	return t.result;
    }

    private static void createAppContext(ThreadGroup tg) {
	AppContextCreatorThread t = new AppContextCreatorThread(tg);
	synchronized(t.synObject) {
	    t.start();
	    try {
		t.synObject.wait();
	    }
	    catch(InterruptedException e) {
	    }
	}
    }
    
    private static boolean isOnWindows() {
	String osName = (String)AccessController.doPrivileged(new sun.security.action.GetPropertyAction("os.name"));
	return (osName.indexOf("Windows") != -1);	
    }

    private static class AppContextCreatorThread extends Thread {
	Object synObject = new Object();
	public AppContextCreatorThread(ThreadGroup tg) {
	    super(tg, "AppContext Creator Thread");
	}

	public void run() {
	    // create new AppContext for the thread group
	    synchronized(synObject) {
		AppContext appCtx = sun.awt.SunToolkit.createNewAppContext();
		pluginEventQueue = (EventQueue)appCtx.get(AppContext.EVENT_QUEUE_KEY);
		synObject.notifyAll();
	    }
	}
    }

    /**
     * SysExecuteThread is a thread that running inside plugin
     * thread group.
     */
    static class SysExecutionThread extends Thread {
	Exception   exception = null;
	Object	    result = null;
	DeploySysAction action = null;
	Object	    syncObject = new Object();
	DummyDialog theDummy = null;

	public SysExecutionThread(DeploySysAction action) {
	    super(pluginThreadGroup, "SysExecutionThead");
	    this.action = action;
	}
	
	public void run() {
	    try {
		result = action.execute();
	    }
	    catch(Exception e) {
		exception = e;
	    }
	    finally {
		if(theDummy != null)
		    theDummy.secureHide();

		synchronized(syncObject) {
		    syncObject.notifyAll();
		}
	    }
	}
    }
}
