/*
 * @(#)ServiceManager.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import com.sun.deploy.util.Trace;


/**
 * ServiceManager is a factory class to obtain different native 
 * code implementation of the service in each platform.
 */
public class ServiceManager
{
    // Browser service for this platform
    private static Service service = new DefaultService();

    /**
     * Returns service for a particular platform.
     *
     * @return Service.
     */
    public synchronized static Service getService()
    {
	return service;
    }

    /**
     * Returns service for a particular platform.
     *
     * @param service Service.
     */
    public synchronized static void setService(int type)
    {
	Class c = DefaultService.class;
	
	try
	{
    	    if (type == PlatformType.STANDALONE_MANTIS_WIN32)
	    {
		c = Class.forName("com.sun.deploy.services.WPlatformService14");
	    }
    	    if (type == PlatformType.STANDALONE_MANTIS_UNIX)
	    {
		c = Class.forName("com.sun.deploy.services.MPlatformService14");
	    }
    	    if (type == PlatformType.STANDALONE_TIGER_WIN32)
	    {
		c = Class.forName("com.sun.deploy.services.WPlatformService");
	    }
    	    if (type == PlatformType.STANDALONE_TIGER_UNIX)
	    {
		c = Class.forName("com.sun.deploy.services.MPlatformService");
	    }
    	    if (type == PlatformType.INTERNET_EXPLORER_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WIExplorerBrowserService");
	    }
	    else if (type == PlatformType.NETSCAPE4_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WNetscape4BrowserService");
	    }
	    else if (type == PlatformType.NETSCAPE6_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WNetscape6BrowserService");
	    }
	    else if (type == PlatformType.NETSCAPE4_UNIX)
	    {
		c = Class.forName("sun.plugin.services.MNetscape4BrowserService");
	    }
	    else if (type == PlatformType.NETSCAPE6_UNIX)
	    {
		c = Class.forName("sun.plugin.services.MNetscape6BrowserService");
	    }
	    else if (type == PlatformType.AXBRIDGE)
	    {
		c = Class.forName("sun.plugin.services.AxBridgeBrowserService");
	    }


    	    service = (Service) c.newInstance();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}
    }

    public synchronized static void setService(Service s) {
        service = s;
    }
}


