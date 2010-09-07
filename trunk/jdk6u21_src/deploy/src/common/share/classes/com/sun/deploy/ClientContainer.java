/*
 * @(#)ClientContainer.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Properties;
import com.sun.deploy.util.Trace;

/**
 * Proxy handler for Direct connection.
 */
public class ClientContainer
{
    /**
     * Main class for the client container.
     */
    public static void main(String[] args)
    {
	if (args.length < 1)
	{
	    System.err.println("Usage: ClientContainer class-name [args ...]");
	    System.exit(1);
	}

	try
	{
	    // Setup services
	    //
	    if (System.getProperty("os.name").indexOf("Windows") != -1)
	    {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_TIGER_WIN32);
	    }	    
	    else
	    {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_TIGER_UNIX);
	    }

	    try
	    {
		com.sun.deploy.net.proxy.DeployProxySelector.reset();

		com.sun.deploy.net.cookie.DeployCookieSelector.reset();
	    }
	    catch(Throwable throwable)
	    {
		// Set up proxy manager
	    
		com.sun.deploy.net.proxy.StaticProxyManager.reset();
	    }


	    // Set up protocol handler
	    //
	    Properties props = System.getProperties();

	    String pkgs = (String) props.get("java.protocol.handler.pkgs");
	    if (pkgs != null) {
		props.put("java.protocol.handler.pkgs", pkgs + "|com.sun.deploy.net.protocol");
	    } else {
		props.put("java.protocol.handler.pkgs", "com.sun.deploy.net.protocol");
	    }
	    System.setProperties(props);
	    
	    // Set up authenticator
	    java.net.Authenticator.setDefault(new com.sun.deploy.security.DeployAuthenticator());

	    // Look up class name
	    Class clazz = Class.forName(args[0]);

	    // Find a static main(String[] args) method
	    Class[] param_type = { (new String[0]).getClass() };

	    Method mainMethod = clazz.getMethod("main", param_type);

	    // Check that method is static
	    if (!Modifier.isStatic(mainMethod.getModifiers())) 
	    {
		throw new NoSuchMethodException("Cannot find main-method.");
	    }

	    mainMethod.setAccessible(true);

	    String[] arguments = new String[args.length - 1];

	    for (int i=0; i < arguments.length; i++)
		arguments[i] = args[i+1];
        
	    Object[] wrappedArguments = { arguments };

	    // Invoke main method
	    mainMethod.invoke(null, wrappedArguments);

	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	    System.exit(2);
	}

	System.exit(0);
    }
}
