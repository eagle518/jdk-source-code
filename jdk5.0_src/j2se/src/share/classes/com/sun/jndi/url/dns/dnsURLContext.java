/*
 * @(#)dnsURLContext.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.dns;


import java.net.MalformedURLException;
import java.util.Hashtable;

import javax.naming.*;
import javax.naming.spi.ResolveResult;
import com.sun.jndi.dns.*;
import com.sun.jndi.toolkit.url.GenericURLDirContext;


/**
 * A DNS URL context resolves names that are DNS pseudo-URLs.
 * See com.sun.jndi.dns.DnsUrl for a description of the URL format.
 *
 * @author Scott Seligman
 * @version 1.7 03/12/19
 */


public class dnsURLContext extends GenericURLDirContext {

    public dnsURLContext(Hashtable env) {
	super(env);
    }

    /**
     * Resolves the host and port of "url" to a root context connected
     * to the named DNS server, and returns the domain name as the
     * remaining name.
     */
    protected ResolveResult getRootURLContext(String url, Hashtable env)
	    throws NamingException {

	DnsUrl dnsUrl;
	try {
	    dnsUrl = new DnsUrl(url);
	} catch (MalformedURLException e) {
	    throw new InvalidNameException(e.getMessage());
	}

	DnsUrl[] urls = new DnsUrl[] { dnsUrl };
	String domain = dnsUrl.getDomain();

	return new ResolveResult(
		DnsContextFactory.getContext(".", urls, env),
		new CompositeName().add(domain));
    }
}
