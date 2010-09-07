/*
 * @(#)JavaWebStartSecurity.java	1.27 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.net.MalformedURLException;
import java.security.*;
import com.sun.deploy.net.CrossDomainXML;
import com.sun.jnlp.JNLPClassLoaderIf;
import com.sun.jnlp.ApiDialog;
import java.util.Vector;
import java.net.URL;


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
    private JNLPClassLoaderIf currentJNLPClassLoader()
    {
	ClassLoader loader;

	// get all the classes on the stack and check them.
	Class[] context = getClassContext();
	for (int i = 0; i < context.length; i++) {
	    loader = context[i].getClassLoader();
	 
	    if (loader instanceof JNLPClassLoaderIf)
		return (JNLPClassLoaderIf)loader;
	}

	// if that fails, try the context class loader
	loader = Thread.currentThread().getContextClassLoader();
	if (loader instanceof JNLPClassLoaderIf)
	    return (JNLPClassLoaderIf)loader;

	return (JNLPClassLoaderIf)null;
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

    private com.sun.jnlp.ApiDialog _connect;

    public void checkConnect(String host, int port) {
	URL url = null;
	int mode;

        // Temporary overloading of checkConnect() behavior for Http handler.
        mode = (port < 0) ? port: CrossDomainXML.CHECK_CONNECT;
        try {
            if (mode == CrossDomainXML.CHECK_SET_HOST || 
                mode == CrossDomainXML.CHECK_SUBPATH) {
                url = new URL(host);
                host = url.getHost();
                port = url.getPort();
                if (port == -1) {
                    port = url.getDefaultPort();
                }
            }
            // See whether we already validated this host 
            if (CrossDomainXML.quickCheck(url, host, port, mode)) {
                return;
            }
        } catch (MalformedURLException mue) {
            // if malformed url - just don't validate by crossdomain.xml
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
            if (_connect == null) {
                _connect = new ApiDialog();
            }
            if (_connect.askConnect(host)) {
                return;
            }
            throw ex;
        }
    }

    public void checkConnect(String host, int port, Object context) {
        URL url = null;
        int mode;

        // Temporary overloading checkConnect() behavior for Http handler.
        mode = (port < 0) ? port: CrossDomainXML.CHECK_CONNECT;
        try {
            if (mode == CrossDomainXML.CHECK_SET_HOST || 
                mode == CrossDomainXML.CHECK_SUBPATH) {
                url = new URL(host);
                host = url.getHost();
                port = url.getPort();
                if (port == -1) {
                    port = url.getDefaultPort();
                }
            }
            // See if we already validated this host via crossdomain.xml
            if (CrossDomainXML.quickCheck(url, host, port, mode)) {
                return;
            }
        } catch (MalformedURLException mue) {
            // fall thru to normal check
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
            // Ask the user to allow connection
            if (_connect == null) {
                _connect = new ApiDialog();
            }
            if (_connect.askConnect(host)) {
                return;
            }
            throw ex;
        }
    }

    private com.sun.jnlp.ApiDialog _accept;

    public void checkAccept(String host, int port) {
	// See if the java.lang.Security.Manager allows the access
	try {
	    super.checkAccept(host, port);
            // the current thread is allowed access, by default.
        } catch (SecurityException ex) {
            // The current thread is not allowed access by default.
	    if (_accept == null) {
		_accept = new ApiDialog();
	    }
	    if (_accept.askAccept(host)) {
		return;
	    }
	    throw ex;
	}
    }
}

