/*
 * @(#)UserProfile.java	1.29 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import java.util.Date;
import java.util.Properties;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.config.Config;


/**
 * UserProfile object encapsulates all the user specific 
 * infrastructure information to be used by Java Plug-in,
 */
public class UserProfile
{
    // Deployment configuration
    private static PluginConfig config = new PluginConfig();

    static
    {
	// Ensure directory is created
	try
	{
	
	    new File(getTempDirectory()).mkdirs();
	}
	catch (Throwable e)
	{   
	    e.printStackTrace();
	}
    }	
        

    /**
     * Returns the plugin property file for this user.
     */
    public static String getPropertyFile()
    {
	return config.getPropertiesFile();
    }
    
    /** 
     * Returns the log directory for this user. 
     */ 
    public static String getLogDirectory() 
    { 
	String logDir = (String) java.security.AccessController.doPrivileged( 
                new sun.security.action.GetPropertyAction("javaplugin.outputfiles.path")); 
  
	if (logDir == null || logDir.trim().equals("")) 
	{ 
	    // Use default 
	    logDir = Config.getLogDirectory(); 
	} 
	
	return logDir;       
    } 

    /**
     * Returns the temp directory for this user.
     */
    public static String getTempDirectory()
    {
	return Cache.getCacheDir().getPath() + File.separator + "tmp";
    }
}

