/*
 * @(#)JavaWebStartSecurity.java	1.18 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.security.*;
import com.sun.jnlp.JNLPClassLoader;
import java.util.Vector;


public
class JavaWebStartSecurity extends SecurityManager {

    /**
     * Construct and initialize.
     */
    public JavaWebStartSecurity() {
    }

    /**
     * get the current (first) instance of an AppletClassLoader on the stack.
     */
    private JNLPClassLoader currentJNLPClassLoader()
    {
	ClassLoader loader;

	// get all the classes on the stack and check them.
	Class[] context = getClassContext();
	for (int i = 0; i < context.length; i++) {
	    loader = context[i].getClassLoader();
	 
	    if (loader instanceof JNLPClassLoader)
		return (JNLPClassLoader)loader;
	}

	// if that fails, try the context class loader
	loader = Thread.currentThread().getContextClassLoader();
	if (loader instanceof JNLPClassLoader)
	    return (JNLPClassLoader)loader;

	// no JNLPClassLoaders on the stack

	return (JNLPClassLoader)null;
    }

    public void checkAwtEventQueueAccess() {

        if (!AppContextUtil.isApplicationAppContext() &&
                (currentJNLPClassLoader() != null)) {

            // If we're about to allow access to an AppContext other than the
            // applications, and anything untrusted is on 
            // the class context stack, then disallow access.

	    super.checkAwtEventQueueAccess();
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

    public void checkPrintJobAccess() {
	// See if the java.lang.Security.Manager allows the print job.
        try {
            super.checkPrintJobAccess();
            // the current thread is allowed to print, by default.
        } catch (SecurityException ex) {
            // The current thread is not allowed to print by default.
	    if (com.sun.jnlp.PrintServiceImpl.requestPrintPermission()) {
	        // success
	        return;
	    } 
	    throw ex;
	}
    }
}

