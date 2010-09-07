/*
 * @(#)DeployURLClassPathCallback.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.security.cert.Certificate;
import java.net.URL;
import java.io.IOException;

/**
 * The class defines the callback methods between a URLClassPath and a
 * handler which manages it's use of class path elements and the
 * resources it returns.
 *
 * @version 1.2 03/24/10
 */
public abstract class DeployURLClassPathCallback {
    abstract public Element openClassPathElement(URL url) throws IOException;
    abstract public Element openClassPathElement(JarFile jf, URL url) throws IOException;


    public static class Element {
	protected JarFile jar;
	protected URL url;

	public Element(JarFile jar, URL url) {
	    this.jar = jar;
	    this.url = url;
	}

	public Element(URL url) {
	    this(null, url);
	}

        /*
         * Called to check and record the trust on a resource about
         * to be returned by URLClassLoader findResource().
         */
        public void checkResource(String name) {
	    throw new SecurityException("checkResource() method not implemented");
	}

	/*
	 * This class path element should be ignored.
	 */
	public boolean skip() {
	    return false;
	}

	/*
	 * This class path element should defer to a child loader instead of
	 * returning a Resource.
	 */
	public boolean defer() {
	    return false;
	}
    }
}

