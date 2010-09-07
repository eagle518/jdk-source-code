/*
 * @(#)AuthCacheBridge.java        07/12/2005
 *
 * Created on July 14, 2005, 8:51 AM
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net.www.protocol.http;

import com.sun.deploy.security.AuthKey;


/**
 * The class is to resolve the problem of getting at private information in the
 * AuthCacheValue class.  The AuthenticationInfo class is private to the
 * sun.net package.  This class acts as an adapter to give the security package
 * in deploy the ability to get at the package private information.  After the
 * class is changed in j2se to have a public interface,  this class will still
 * be needed for Web Start running with the 1.5 JRE
 *
 * @author Ashley Woodsom
 */
public class AuthCacheBridge implements AuthKey {
    AuthenticationInfo cacheValue;

    /** Creates a new instance of AuthCacheBridge */
    public AuthCacheBridge(AuthCacheValue value) {
        cacheValue = (AuthenticationInfo) value;
    }

    /**
     * Returns whether the requestor is a Proxy or a Server.
     *
     * @return true if connection is with a proxy server
     */
    public boolean isProxy() {
        return (cacheValue.getAuthType() == AuthCacheValue.Type.Proxy);
    }

    /**
     * Give the protocol that's requesting the connection.  I.E. http, https
     *
     * @return the protcol
     */
    public String getProtocolScheme() {
        return cacheValue.getProtocolScheme();
    }

    /**
     * Gets the port number for the requested connection.
     * @return an <code>int</code> indicating the
     * port for the requested connection.
     */
    public int getPort() {
        return cacheValue.getPort();
    }

    /**
     * Gets the hostname of the
     * site or proxy requesting authentication, or null
     * if not available.
     *
     * @return the hostname of the connection requiring authentication, or null
     *                if it's not available.
     */
    public String getHost() {
        return cacheValue.getHost();
    }

    /**
     * Gets the path on the server
     *
     * @return the path
     */
    public String getPath() {
        return cacheValue.getPath();
    }
}
