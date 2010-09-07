/*
 * @(#)JNLPClassLoaderUtil.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.JarURLConnection;
import java.util.jar.JarFile;
import java.util.ArrayList;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.CodeSource;
import java.security.CodeSigner;
import java.security.Permission;
import java.security.PermissionCollection;
import com.sun.deploy.util.Trace;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.security.AppPolicy;

/**
 * This is the common abstract base for the JNLP classloader 
 *
 */
public class JNLPClassLoaderUtil 
{
    /** 
     * Get instance 
     *
     * We maintain compatibility with the original singular 
     * JNLPClassLoader.getInstance() pattern (core JAVAWS),
     * if the current threads class-loader is not of type JNLPClassLoaderIf (JNLP-OOPP)!
     */
    public static JNLPClassLoaderIf getInstance() 
    {
        // if this Thread's ClassLoader is not the new JNLPClassLoaderIf implementation (JNLP-OOPP),
        // fetch the static core-JAVAWS one.
        ClassLoader loader = Thread.currentThread().getContextClassLoader();
        if (loader instanceof JNLPClassLoaderIf)
        {
            return (JNLPClassLoaderIf)loader;
        }

        JNLPClassLoaderIf jnlpLoader = JNLPClassLoader.getInstance(); 

        if ( jnlpLoader == null )
        {
            Trace.ignoredException(new Exception("JNLPClassLoaderUtil: couldn't find a valid JNLPClassLoaderIf"));
        }
        return jnlpLoader;
    }
}


