/*
 * @(#)VersionHelper.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.net.MalformedURLException;
import java.net.URL;

abstract class VersionHelper {

    private static VersionHelper helper = null;

    VersionHelper() {} // Disallow anyone from creating one of these.

    static {
	try {
	    Class.forName("java.net.URLClassLoader"); // 1.2 test
	    Class.forName("java.security.PrivilegedAction"); // 1.2 test
	    helper = (VersionHelper)
		Class.forName(
		    "com.sun.jndi.ldap.VersionHelper12").newInstance();
	} catch (Exception e) {
	}

	// Use 1.1 helper if 1.2 test fails, or if we cannot create 1.2 helper
	if (helper == null) {
	    try {
		helper = (VersionHelper)
		    Class.forName(
			"com.sun.jndi.ldap.VersionHelper11").newInstance();
	    } catch (Exception e) {
		// should never happen
	    }
	}
    }

    static VersionHelper getVersionHelper() {
	return helper;
    }

    abstract ClassLoader getURLClassLoader(String[] url) 
	throws MalformedURLException;


    static protected URL[] getUrlArray(String[] url) throws MalformedURLException {
	URL[] urlArray = new URL[url.length];
	for (int i = 0; i < urlArray.length; i++) {
	    urlArray[i] = new URL(url[i]);
	}
	return urlArray;
    }

    abstract Class loadClass(String className) throws ClassNotFoundException;

    abstract Thread createThread(Runnable r);
}

