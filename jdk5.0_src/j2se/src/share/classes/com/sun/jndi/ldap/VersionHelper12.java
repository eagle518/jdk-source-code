/*
 * @(#)VersionHelper12.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.net.URL;
import java.net.URLClassLoader;
import java.net.MalformedURLException;
import java.security.AccessController;
import java.security.PrivilegedAction;

final class VersionHelper12 extends VersionHelper {

    VersionHelper12() {} // Disallow external from creating one of these.

    ClassLoader getURLClassLoader(String[] url) 
	throws MalformedURLException {
	    ClassLoader parent = getContextClassLoader();
	    if (url != null) {
		return URLClassLoader.newInstance(getUrlArray(url), parent);
	    } else {
		return parent;
	    }
    }

    Class loadClass(String className) throws ClassNotFoundException {
	ClassLoader cl = getContextClassLoader();
	return Class.forName(className, true, cl);
    }

    private ClassLoader getContextClassLoader() {
	return (ClassLoader) AccessController.doPrivileged(
	    new PrivilegedAction() {
		public Object run() {
		    return Thread.currentThread().getContextClassLoader();
		}
	    }
	);
    }

    Thread createThread(final Runnable r) {
	return (Thread) AccessController.doPrivileged(
	    new PrivilegedAction() {
	        public Object run() {
		    return new Thread(r);
		}
	    }
	);
    }
}
