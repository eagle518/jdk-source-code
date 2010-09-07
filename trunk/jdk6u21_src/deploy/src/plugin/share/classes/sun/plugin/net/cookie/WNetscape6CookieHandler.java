/*
 * @(#)WNetscape6CookieHandler.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;
import com.sun.deploy.net.cookie.CookieHandler;
import com.sun.deploy.net.cookie.CookieUnavailableException;
import sun.plugin.viewer.AppletPanelCache;


/**
 * <p> WNetscape6CookieHandler is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL in Netscape 6 on Win32.
 * </p>
 */
public final class WNetscape6CookieHandler implements CookieHandler 
{
    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @param String contains the corresponding cookie value.
     */
    public void setCookieInfo(URL url, String value)
		throws CookieUnavailableException
    {
	// Cookie service in Netscape 6 is only available if
	// there is at least one plugin instance exists. Otherwise,
	// accessing the cookie service without any plugin instance
	// will likely to confuse the browser and fail. Thus, it 
	// is VERY important to check if there is at least one 
	// applet instance before calling back to the browser.
	//

	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Cookie service is NOT available
	    throw new CookieUnavailableException("Cookie service is not available for " + url);
	}	

	nativeSetCookieInfo(url.toString(), value);
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public String getCookieInfo(URL url)
		  throws CookieUnavailableException
    {
	// Cookie service in Netscape 6 is only available if
	// there is at least one plugin instance exists. Otherwise,
	// accessing the cookie service without any plugin instance
	// will likely to confuse the browser and fail. Thus, it 
	// is VERY important to check if there is at least one 
	// applet instance before calling back to the browser.
	//

	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Cookie service is NOT available
	    throw new CookieUnavailableException("Cookie service is not available for " + url);
	}	

	return nativeGetCookieInfo(url.toString());
    }

    public native void nativeSetCookieInfo(String u, String value);
    public native String nativeGetCookieInfo(String url);
}


