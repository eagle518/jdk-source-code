/*
 * @(#)IExplorerCookieHandler.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.net.URL;

/**
 * <p> IExplorerCookieHandler is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL in Internet Explorer.
 * </p>
 */
public final class IExplorerCookieHandler implements com.sun.deploy.net.cookie.CookieHandler 
{
    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @param String contains the corresponding cookie value.
     */
    public void setCookieInfo(URL url, String value)
    {
	setCookieInfo(url.toString(), value);
    }

    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @param String contains the corresponding cookie value.
     */
    public native void setCookieInfo(String url, String value);

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public String getCookieInfo(URL url)
    {
	return getCookieInfo(url.toString());
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public native String getCookieInfo(String url);
}


