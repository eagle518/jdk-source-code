/*
 * @(#)SessionCookieStore.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;


/**
 * SessionCookieStore class represents an in-memory storage for 
 * sessioin cookie. Can record, retrieve, and store cookies associated 
 * with particular URLs.
 */
final class SessionCookieStore extends CookieStore
{
    /**
     * Load cookie jar from storage.
     */    
    protected void loadCookieJar()
    {
	// no-op
    }

    /**
     * Save cookie jar into storage.
     */    
    protected void saveCookieJar()
    {
	// no-op
    }

    /** 
     * Return cookie store name.
     */
    protected String getName()
    {
	return "Session Cookie Store";
    }

    /**
     * Predicate function which returns true if the cookie appears to be
     * invalid somehow and should not be added to the cookie set.
     */
    protected boolean shouldRejectCookie(HttpCookie cookie) 
    {
	// Call super
	if (super.shouldRejectCookie(cookie))
	    return true;

	// Session cookie has no expiration date.
	if (cookie.getExpirationDate() != null)
	    return true;
	else
	    return false;
    }
}

