/*
 * @(#)AuthCacheValue.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.io.IOException;
import java.io.Serializable;
import java.net.*;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Enumeration;
import java.util.HashMap;


/**
 * AuthCacheValue: interface to minimise exposure to authentication cache
 * for external users (ie. plugin)
 *
 * @author Michael McMahon
 * @version 1.1, 10/02/03 
 */

public abstract class AuthCacheValue implements Serializable {

    public enum Type {
	Proxy,
	Server
    };

    /**
     * Caches authentication info entered by user.  See cacheKey()
     */
    static protected AuthCache cache = new AuthCacheImpl();

    public static void setAuthCache (AuthCache map) {
	cache = map;
    }

    /* Package private ctor to prevent extension outside package */

    AuthCacheValue() {}

    abstract Type getAuthType ();

   /**
    * name of server/proxy
    */
    abstract String getHost ();

   /**
    * portnumber of server/proxy
    */
    abstract int getPort();

   /**
    * realm of authentication if known
    */
    abstract String getRealm();

    /**
     * root path of realm or the request path if the root
     * is not known yet.
     */
    abstract String getPath();

    /**
     * returns http or https
     */
    abstract String getProtocolScheme();

    /**
     * the credentials associated with this authentication
     */
    abstract PasswordAuthentication credentials();
}
