/*
 * @(#)CookieHandler.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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



