/*
 * @(#)DeployRMIClassLoaderSpi.java	1.2 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.util;

import java.net.MalformedURLException;
import java.rmi.server.RMIClassLoaderSpi;
import java.rmi.server.RMIClassLoader;
import java.security.AccessController;
import java.security.PrivilegedAction;

public abstract class DeployRMIClassLoaderSpi extends RMIClassLoaderSpi {

    public DeployRMIClassLoaderSpi() {
        super();
    }
    
    protected static RMIClassLoaderSpi getDefaultProviderInstance() {
        return (RMIClassLoaderSpi) AccessController.doPrivileged(
                new PrivilegedAction() {

                    public Object run() {
                        return RMIClassLoader.getDefaultProviderInstance();
                    }
                });
    }

    public Class loadProxyClass(String codebase,
            String[] interfaces,
            ClassLoader defaultLoader)
            throws MalformedURLException,
            ClassNotFoundException {

        return getDefaultProviderInstance().loadProxyClass(codebase,
                interfaces, defaultLoader);

    }

    public Class loadClass(String codebase,
            String name,
            ClassLoader defaultLoader)
            throws MalformedURLException,
            ClassNotFoundException {
        return getDefaultProviderInstance().loadClass(codebase, name, 
                defaultLoader);
    }

    public ClassLoader getClassLoader(String codebase)
            throws MalformedURLException {
        return getDefaultProviderInstance().getClassLoader(codebase);
    }

    public String getClassAnnotation(Class cl) {
        if (useRMIServerCodebaseForClass(cl)) {
            return (String) AccessController.doPrivileged(new PrivilegedAction() {

                public Object run() {
                    return System.getProperty("java.rmi.server.codebase");
                }
            });
        }
        return getDefaultProviderInstance().getClassAnnotation(cl);
    }

    protected abstract boolean useRMIServerCodebaseForClass(Class cl); 
}
