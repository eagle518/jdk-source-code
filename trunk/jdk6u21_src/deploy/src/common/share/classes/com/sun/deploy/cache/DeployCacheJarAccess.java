/*
 * @(#)DeployCacheJarAccess.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.jar.JarFile;
import java.security.CodeSource;
import java.security.cert.Certificate;

public interface DeployCacheJarAccess {
    public Enumeration entryNames(JarFile jar, CodeSource[] cs);
    public CodeSource[] getCodeSources(JarFile jar, URL url);
    public CodeSource getCodeSource(JarFile jar, URL url, String name);
    public void setEagerValidation(JarFile jar, boolean eager);
}

