/*
 * @(#)AuthKey.java        07/12/2005
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.security;


/**
 * This interface defines a common set of methods for mapping connections
 * to authentication credentials.
 *
 * @author Ashley Woodsom
 */
public interface AuthKey {
    /**
     * Returns whether the requestor is a Proxy
     *
     * @return true if connection is with a proxy server
     */
    public abstract boolean isProxy();

    /**
     * Give the protocol that's requesting the connection.  I.E. http, https
     *
     * @return the protcol
     */
    public abstract String getProtocolScheme();

    /**
     * Gets the <code>hostname</code> of the
     * site or proxy requesting authentication, or <code>null</code>
     * if not available.
     *
     * @return the hostname of the connection requiring authentication, or null
     *                if it's not available.
     */
    public abstract String getHost();

    /**
     * Gets the port number for the requested connection.
     * @return an <code>int</code> indicating the
     * port for the requested connection.
     */
    public abstract int getPort();

    /**
     * Gets the path on the host the credential is required for
     *
     * @return the path I.E. /games/cards/mygame.jnlp
     */
    public abstract String getPath();
}
