/*
 * @(#)CookieHandler.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.net.URL;


public interface CookieHandler 
{
    /* 
     * getCookieInfo takes a particular URL and returns its cookie info.
     */
    String getCookieInfo(URL url) throws CookieUnavailableException;

    /* 
     * setCookieInfo takes a particular URL and its cookie info.
     */
    void setCookieInfo(URL url, String value) throws CookieUnavailableException;
}



