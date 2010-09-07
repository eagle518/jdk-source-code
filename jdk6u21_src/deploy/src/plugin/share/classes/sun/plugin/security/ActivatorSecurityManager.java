/*
 * @(#)ActivatorSecurityManager.java	1.46 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Set;
import java.util.HashSet;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import sun.awt.AppContext;
import sun.plugin.util.PluginSysUtil;
import com.sun.deploy.net.CrossDomainXML;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;

/** 
 * The Activator security manager.
 *
 * This extends and makes very small tweaks to the standard
 * Applet security manager
 */

public class ActivatorSecurityManager extends sun.applet.AppletSecurity {

    private final Set lockedThreadGroup = new HashSet();

    public ActivatorSecurityManager() { 
    }

    /**
     * Checks to see if an applet can get EventQueue access.
     * Work around oversight in normal applet security manager.
     */
    public void checkAwtEventQueueAccess() {
	// See if the java.lang.Security.Manager allows it.
	// This always fails on 1.1, but may pass on 1.2
	try {
	    super.checkAwtEventQueueAccess();
	    // the current thread is allowed access.
	    return;
	} catch (SecurityException ex) {
	    // Drop through.
	}
	// This passes on 1.1 if we are trusted.
	// It gets correctly checked on 1.2
	checkSecurityAccess("accessEventQueue");
    }

    /**
     * Checks to see if an applet can perform a given operation.
     */
    public void checkSecurityAccess(String action) {
	if (action != null && action.equals("java"))
	    return;
	else
	    super.checkSecurityAccess(action);
    }


    /**
     * Tests if a client can initiate a print job request.
     *
     * If our superclass grants permission, so do we.
     *
     * If our superclass denies permission, we pop up a Dialog
     * and ask the user if they wish to allow the print job.
     *
     */
    public void checkPrintJobAccess() {

	// See if the java.lang.Security.Manager allows the print job.
	try {
	    super.checkPrintJobAccess();
	    // the current thread is allowed to print, by default.
	    // return success.
	    return;
	} catch (SecurityException ex) {
	    // The current thread is not allowed to print by default.
	    // Drop through and pop up a dialog.
	}

	// version for JDK 1.2 and later
	// We have to push things off to a separate class (CheckPrint_1_2)
	// here to avoid verifiers errors on 1.1.
	//
	// Constructing the CheckPrint_1_2 class causes it
	// to run a privileged clostrue that pops up the Dialog.
	new CheckPrint_1_2();
    }

    private class CheckPrint_1_2 implements java.security.PrivilegedAction {
	CheckPrint_1_2() {
	    // Run a privileged closure to bring up a Dialog box
	    java.security.AccessController.doPrivileged(this);
	}
	public Object run() {
	    showPrintDialog();
	    return null;
	}
    }


    // Will throw SecurityException if it's not OK to print.
    void showPrintDialog() {
        AppContext context = AppContext.getAppContext();
	String title = ResourceManager.getString("plugin.print.title");
	String message = ResourceManager.getString("plugin.print.message");
	String checkBoxStr = ResourceManager.getString("plugin.print.always");

	String printFlag = 
            (String)context.get("sun.plugin.security.printDialog");

	int result = 0;
	
	// Check if automation is enabled
	if (Trace.isAutomationEnabled() == false && printFlag == null)
	{
            result = UIFactory.showApiDialog(null, null, title, message,
		null, null, checkBoxStr, false);
	} else {
	    Trace.msgSecurityPrintln("securitymgr.automation.printing");
	    result = UIFactory.OK;
	}

	if (result == UIFactory.ALWAYS) {
	    context.put("sun.plugin.security.printDialog","skip");
	} else if (result != UIFactory.OK) {
            throw new SecurityException("checkPrintJobAccess");
        }
    }

    /**
     * getExecutionStackContext returns all the classes that are
     * on the current execution stack.
     *
     * @return Class object array
     */	
    public Class[] getExecutionStackContext()
    {
	return super.getClassContext();
    }

    public synchronized void checkAccess(ThreadGroup g) {
        super.checkAccess(g);

        if (g.parentOf(Thread.currentThread().getThreadGroup())) {
            if (lockedThreadGroup.contains(g)) {
                throw new IllegalThreadStateException("forbid thread creation in disposed TG");
            }
        }
    }

    public synchronized void lockThreadGroup(ThreadGroup g) {
        if (g != null) {
            lockedThreadGroup.add(g);
        }
    }

    public synchronized void unlockThreadGroup(ThreadGroup g) {
        if (g != null) {
            lockedThreadGroup.remove(g);
        }
    }

    public void checkConnect(String host, int port) {
        URL url = null;
        int mode;

        // Temporary overloading of checkConnect() behavior for Http handler.
        mode = (port < 0) ? port: CrossDomainXML.CHECK_CONNECT;
        if (mode == CrossDomainXML.CHECK_SET_HOST || 
            mode == CrossDomainXML.CHECK_SUBPATH) {
            try {
                url = new URL(host);
                host = url.getHost();
                port = url.getPort();
                if (port == -1) {
                    port = url.getDefaultPort();
                }
                // See if we already validated this host via crossdomain.xml
                if (CrossDomainXML.quickCheck(url, host, port, mode)) {
                    return;
                }
            } catch (MalformedURLException ex) {
                // fall thru to normal check
            }
        }

        // See if the java.lang.Security.Manager allows the access
        try {
            super.checkConnect(host, port);
            // the current thread is allowed access, by default.
        } catch (SecurityException ex) {
            // The current thread is not allowed access by default.

            // See whether the crossdomain.xml allows access
            // removing check for url == null, since when mode is not either
            // CHECK_SET_HOST or CHECK_SUBPATH, url will be null, 
            if (CrossDomainXML.check(getClassContext(), 
                                     url, host, port, mode)) {
                return;
            }
            throw ex;
        }
    }

    public void checkConnect(String host, int port, Object context) {
        URL url = null;
        int mode;

        // Temporary overloading of checkConnect() behavior for Http handler.
        mode = (port < 0) ? port: CrossDomainXML.CHECK_CONNECT;
        if (mode == CrossDomainXML.CHECK_SET_HOST || 
            mode == CrossDomainXML.CHECK_SUBPATH) {
            try {
                url = new URL(host);
                host = url.getHost();
                port = url.getPort();
                if (port == -1) {
                    port = url.getDefaultPort();
                }
                // See if we already validated this host via crossdomain.xml
                if (CrossDomainXML.quickCheck(url, host, port, mode)) {
                    return;
                }
            } catch (MalformedURLException ex) {
                // fall through to normal check
            }
        }

        // See if the java.lang.Security.Manager allows the access
        try {
            super.checkConnect(host, port, context);
            // the current thread is allowed access, by default.
        } catch (SecurityException ex) {
            // The current thread is not allowed access by default.
            // See whether the crossdomain.xml allows access

            // removing check for url == null, since when mode is not either
            // CHECK_SET_HOST or CHECK_SUBPATH, url will be null,
            if (CrossDomainXML.check(getClassContext(), 
                                     url, host, port, mode)) {
                return;
            }
            throw ex;
        }
    }
}


