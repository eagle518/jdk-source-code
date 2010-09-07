/*
 * @(#)BridgeFactory.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

import java.applet.Applet;
import java.security.*;

/** Provides {@link Bridge Bridge} instances to runtimes for non-Java
    languages hosted on the Java platform. */

public final class BridgeFactory {
    private BridgeFactory() {}

    /** Fetches the {@link Bridge Bridge} instance for the given
        applet. A new Bridge instance is created for each new applet
        instance, so new {@link InvocationDelegate InvocationDelegate}
        and {@link ConversionDelegate ConversionDelegate} instances
        must be registered for each new applet. This registration
        should typically be done in the applet's {@link Applet#init
        init} method. <P>

        First, if there is a security manager, its {@link
        SecurityManager#checkPermission(Permission) checkPermission}
        method is called with a {@code
        RuntimePermission("liveconnect.accessBridge")}
        permission. This check implies that the code for the language
        runtime must be both signed and trusted. A {@code
        SecurityException} is raised if the calling code does not have
        this permission. <P>

        {@link Bridge Bridge} instances must not be passed to
        untrusted code.

        @param applet the applet whose Bridge is being fetched
        @return the Bridge instance for the given applet
        @throws IllegalArgumentException if the given applet is not
                currently running
        @throws SecurityException if the caller does not have the
                {@code liveconnect.accessBridge} RuntimePermission
    */
    public static Bridge getBridge(Applet applet) throws IllegalArgumentException, SecurityException {
        SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(new java.lang.RuntimePermission("liveconnect.accessBridge"));
        }
        return sun.plugin2.main.client.LiveConnectSupport.getBridge(applet);
    }
}
