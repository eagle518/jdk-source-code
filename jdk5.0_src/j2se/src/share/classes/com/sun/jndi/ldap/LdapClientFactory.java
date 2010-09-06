/*
 * @(#)LdapClientFactory.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.io.OutputStream;
import javax.naming.InterruptedNamingException;
import javax.naming.CommunicationException;
import javax.naming.NamingException;

import com.sun.jndi.ldap.pool.PoolCallback;
import com.sun.jndi.ldap.pool.PooledConnection;
import com.sun.jndi.ldap.pool.PooledConnectionFactory;

/**
 * Creates an LdapClient. Encapsulates the parameters required to create 
 * an LdapClient and provides methods for returning appropriate exceptions
 * to throw when acquiring a pooled LdapClient fails.
 *
 * @author Rosanna Lee
 */
final class LdapClientFactory implements PooledConnectionFactory {
    final private String host;
    final private int port;
    final private String socketFactory;
    final private int timeout;
    final private OutputStream trace;

    LdapClientFactory(String host, int port, String socketFactory,
	int timeout, OutputStream trace) {
	this.host = host;
	this.port = port;
	this.socketFactory = socketFactory;
	this.timeout = timeout;
	this.trace = trace;
    }

    public PooledConnection createPooledConnection(PoolCallback pcb) 
	throws NamingException {
	return new LdapClient(host, port, socketFactory, timeout, trace, pcb);
    }

    public String toString() {
	return host + ":" + port;
    }
}
