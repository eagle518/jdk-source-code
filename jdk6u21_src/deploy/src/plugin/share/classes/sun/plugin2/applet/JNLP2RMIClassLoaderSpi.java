/*
 * @(#)JNLP2RMIClassLoaderSpi.java	1.2 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin2.applet;

import com.sun.deploy.util.DeployRMIClassLoaderSpi;
import java.rmi.server.RMIClassLoader;
import java.security.AccessController;
import java.security.PrivilegedAction;

public final class JNLP2RMIClassLoaderSpi extends DeployRMIClassLoaderSpi {

    public JNLP2RMIClassLoaderSpi() {
        super();
    }

    // for class that is loaded by JNLP2ClassLoader, 
    // override getClassAnnotation to always return value of
    // java system property java.rmi.server.codebase
    // this is to prevent the default provider to call into
    // JNLP2ClassLoader.getURLs and return the URLs to all the JARs listed
    // in the applet JNLP file instead, which will significantly increase the 
    // number of packets transferred for each RMI call made by JNLP applets
    protected boolean useRMIServerCodebaseForClass(Class cl) {
        return (cl != null && cl.getClassLoader() instanceof JNLP2ClassLoader);
    } 
}
