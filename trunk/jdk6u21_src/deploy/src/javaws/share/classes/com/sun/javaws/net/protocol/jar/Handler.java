/*
 * @(#)Handler.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.net.protocol.jar;

import java.net.URL;
import java.io.IOException;
import com.sun.jnlp.JNLPCachedJarURLConnection;

/*
 * Jar URL Handler for Java Web Start
 */
public class Handler extends sun.net.www.protocol.jar.Handler
{
    protected java.net.URLConnection openConnection(URL u)
    throws IOException {
	return new JNLPCachedJarURLConnection(u, this);
    }
}

