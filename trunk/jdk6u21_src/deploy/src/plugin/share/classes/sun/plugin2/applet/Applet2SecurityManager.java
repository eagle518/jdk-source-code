/*
 * @(#)Applet2SecurityManager.java	1.9 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.io.FileDescriptor;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.net.SocketPermission;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.HashSet;
import java.util.StringTokenizer;
import java.security.*;
import java.lang.reflect.*;
import sun.awt.AWTSecurityManager;
import sun.awt.AppContext;
import sun.security.provider.*;
import sun.security.util.SecurityConstants;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import sun.plugin.util.PluginSysUtil;
import com.sun.deploy.net.CrossDomainXML;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;

/**
 * This class defines the applet's security policy. It folds together
 * the original AppletSecurity as well as the ActivatorSecurityManager.
 * It must be cloned because the original AppletSecurity was tied to
 * the AppletClassLoader, which is not used in the new applet
 * implementation. FIXME: it would be better if the algorithms used in
 * this SecurityManager were not reliant on the specific class loader
 * class.
 */
public
class Applet2SecurityManager extends AWTSecurityManager {
    private AppContext mainAppContext;

    //URLClassLoader.acc
    private static Field facc = null;

    //AccessControlContext.context;
    private static Field fcontext = null;

    static {
        try {
            facc = URLClassLoader.class.getDeclaredField("acc");
            facc.setAccessible(true);
            fcontext = AccessControlContext.class.getDeclaredField("context");
            fcontext.setAccessible(true);
        } catch (NoSuchFieldException e) {
            throw new UnsupportedOperationException(e);
        }
    }


    /**
     * Construct and initialize.
     */
    public Applet2SecurityManager() {
	reset();
	mainAppContext = AppContext.getAppContext();
    }

    // Cache to store known restricted packages
    private HashSet restrictedPackages = new HashSet();
   
    /**
     * Reset from Properties
     */
    public void reset() 
    {
	// Clear cache
	restrictedPackages.clear();

	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() 
	    {
		// Enumerate system properties
		Enumeration e = System.getProperties().propertyNames();

		while (e.hasMoreElements())
		{
		    String name = (String) e.nextElement();

		    if (name != null && name.startsWith("package.restrict.access."))
		    {
			String value = System.getProperty(name);

			if (value != null && value.equalsIgnoreCase("true"))
			{
			    String pkg = name.substring(24);

    			    // Cache restricted packages
			    restrictedPackages.add(pkg);
			}
		    }
		}
		return null;
	    }
	});
    }

    /**
     * get the current (first) instance of an Plugin2ClassLoader on the stack.
     */
    private Plugin2ClassLoader currentAppletClassLoader()
    {
	// try currentClassLoader first
	ClassLoader loader = currentClassLoader();
	
	if ((loader == null) || (loader instanceof Plugin2ClassLoader))
	    return (Plugin2ClassLoader)loader;

	// if that fails, get all the classes on the stack and check them.
	Class[] context = getClassContext();
	for (int i = 0; i < context.length; i++) {
	    loader = context[i].getClassLoader();
	    if (loader instanceof Plugin2ClassLoader)
		return (Plugin2ClassLoader)loader;
	}

	/* 
	 * fix bug # 6433620 the logic here is : try to find URLClassLoader from
	 * class context, check its AccessControlContext to see if
	 * Plugin2ClassLoader is in stack when it's created. for this kind of
	 * URLClassLoader, return the AppContext assocated with the
	 * Plugin2ClassLoader.
	 */
	for (int i = 0; i < context.length; i++) {
	    final ClassLoader currentLoader = context[i].getClassLoader();

	    if (currentLoader instanceof URLClassLoader) {
		loader = (ClassLoader) AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {

			AccessControlContext acc = null;
			ProtectionDomain[] pds = null;

			try {
			    acc = (AccessControlContext) facc.get(currentLoader);
			    if (acc == null) {
				return null;
			    }

		    	    pds = (ProtectionDomain[]) fcontext.get(acc);
			    if (pds == null) {
				return null;
			    }
			} catch (Exception e) {
            		    throw new UnsupportedOperationException(e);
			}

		    	for (int i=0; i<pds.length; i++) {
		    	    ClassLoader cl = pds[i].getClassLoader();

	 		    if (cl instanceof Plugin2ClassLoader) {
				    return cl;
			    }
			}

			return null;
	    	    }
		});

		if (loader != null) {
		    return (Plugin2ClassLoader) loader;
		}
	    }
	}

	// if that fails, try the context class loader
	loader = Thread.currentThread().getContextClassLoader();
	if (loader instanceof Plugin2ClassLoader)
	    return (Plugin2ClassLoader)loader;

	// no Plugin2ClassLoader on the stack
	return (Plugin2ClassLoader)null;
    }

    /**
     * Returns true if this threadgroup is in the applet's own thread
     * group. This will return false if there is no current class
     * loader.
     */
    protected boolean inThreadGroup(ThreadGroup g) {
	if (currentAppletClassLoader() == null)
	    return false;
	else
	    return getThreadGroup().parentOf(g);
    }

    /**
     * Returns true of the threadgroup of thread is in the applet's
     * own threadgroup.
     */
    protected boolean inThreadGroup(Thread thread) {
	return inThreadGroup(thread.getThreadGroup());
    }

    /**
     * Applets are not allowed to manipulate threads outside
     * applet thread groups. However a terminated thread no longer belongs
     * to any group.
     */
    public void checkAccess(Thread t) {
        /* When multiple applets is reloaded simultaneously, there will be 
         * multiple invocations to this method from plugin's SecurityManager. 
         * This method should not be synchronized to avoid deadlock when 
         * a page with multiple applets is reloaded 
         */
	if (!isThreadTerminated(t) && !inThreadGroup(t)) {
	    checkPermission(SecurityConstants.MODIFY_THREAD_PERMISSION);
	}
    }

    private static boolean isThreadTerminated(Thread t) {
        // Thread.getState() was added in JDK 5, but this code also needs to run on JDK 1.4.2
        try {
            return (t.getState() == Thread.State.TERMINATED);
        } catch (Throwable e) {
            return (!t.isAlive());
        }
    }

    private boolean inThreadGroupCheck = false;

    /**
     * Applets are not allowed to manipulate thread groups outside
     * applet thread groups.
     */
    public synchronized void checkAccess(ThreadGroup g) {
	if (inThreadGroupCheck) {
	    // if we are in a recursive check, it is because
	    // inThreadGroup is calling appletLoader.getThreadGroup
	    // in that case, only do the super check, as appletLoader
	    // has a begin/endPrivileged
	    checkPermission(SecurityConstants.MODIFY_THREADGROUP_PERMISSION);
	} else {
	    try {
		inThreadGroupCheck = true;
		if (!inThreadGroup(g)) {
		    checkPermission(SecurityConstants.MODIFY_THREADGROUP_PERMISSION);
		}
	    } finally {
		inThreadGroupCheck = false;
	    }
	}
    }


    /**
     * Throws a <code>SecurityException</code> if the 
     * calling thread is not allowed to access the package specified by 
     * the argument. 
     * <p>
     * This method is used by the <code>loadClass</code> method of class 
     * loaders. 
     * <p>
     * The <code>checkPackageAccess</code> method for class 
     * <code>SecurityManager</code>  calls
     * <code>checkPermission</code> with the
     * <code>RuntimePermission("accessClassInPackage."+pkg)</code>
     * permission.
     *
     * @param      pkg   the package name.
     * @exception  SecurityException  if the caller does not have
     *             permission to access the specified package.
     * @see        java.lang.ClassLoader#loadClass(java.lang.String, boolean)
     */
    public void checkPackageAccess(final String pkgname) {

	// first see if the VM-wide policy allows access to this package
	super.checkPackageAccess(pkgname);

    // FIXME: restrict access to the JNLP services to JNLP applets
    // IF ( ! currentAppletClassLoader() instanceof JNLP2ClassLoader )
    //  THEN forbid javax.jnlp.*

	// now check the list of restricted packages
	for (Iterator iter = restrictedPackages.iterator(); iter.hasNext();)
	{
	    String pkg = (String) iter.next();
	    
	    // Prevent matching "sun" and "sunir" even if they
	    // starts with similar beginning characters
	    //
	    if (pkgname.equals(pkg) || pkgname.startsWith(pkg + "."))
	    {
		checkPermission(new java.lang.RuntimePermission
			    ("accessClassInPackage." + pkgname));
	    }
	}
    }

    /**
     * Tests if a client can get access to the AWT event queue.
     * <p>
     * This method calls <code>checkPermission</code> with the
     * <code>AWTPermission("accessEventQueue")</code> permission.
     *
     * @since   JDK1.1
     * @exception  SecurityException  if the caller does not have 
     *             permission to accesss the AWT event queue.
     */
    public void checkAwtEventQueueAccess() {
	// See if the java.lang.Security.Manager allows it.
	// This always fails on 1.1, but may pass on 1.2
	try {
            AppContext appContext = AppContext.getAppContext();
            Plugin2ClassLoader appletClassLoader = currentAppletClassLoader();

            if ((appContext == mainAppContext) && (appletClassLoader != null)) {
                // If we're about to allow access to the main EventQueue,
                // and anything untrusted is on the class context stack,
                // disallow access.
                super.checkAwtEventQueueAccess();
            }
	    // the current thread is allowed access.
	    return;
	} catch (SecurityException ex) {
	    // Drop through.
	}
	// This passes on 1.1 if we are trusted.
	// It gets correctly checked on 1.2
	checkSecurityAccess("accessEventQueue");
    } // checkAwtEventQueueAccess()

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
     * Returns the thread group of the applet. We consult the classloader
     * if there is one.
     */
    public ThreadGroup getThreadGroup() {
        /* If any applet code is on the execution stack, we return
           that applet's ThreadGroup.  Otherwise, we use the default
           behavior. */
        Plugin2ClassLoader appletLoader = currentAppletClassLoader();
        ThreadGroup loaderGroup = (appletLoader == null) ? null
                                          : appletLoader.getThreadGroup();
        if (loaderGroup != null) {
            return loaderGroup;
        } else {
            return super.getThreadGroup();
        }
    } // getThreadGroup()

    /**
      * Get the AppContext corresponding to the current context.
      * The default implementation returns null, but this method
      * may be overridden by various SecurityManagers
      * (e.g. Applet2SecurityManager) to index AppContext objects by
      * the calling context.
      * 
      * @return  the AppContext corresponding to the current context.
      * @see     sun.awt.AppContext
      * @see     java.lang.SecurityManager
      * @since   JDK1.2.1
      */
    public AppContext getAppContext() {
        Plugin2ClassLoader appletLoader = currentAppletClassLoader();

	if (appletLoader == null) {
	    return null;
	} else {
	    AppContext context =  appletLoader.getAppContext();
		
	    // context == null when some thread in applet thread group
	    // has not been destroyed in AppContext.dispose() 
	    if (context == null) {
		throw new SecurityException("Applet classloader has invalid AppContext");
	    }

	    return context;
	}
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

    // NOTE: getExecutionStackContext() elided -- hope to avoid reintroducing it.

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
                // fall through to normal check
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
} // class Applet2SecurityManager
