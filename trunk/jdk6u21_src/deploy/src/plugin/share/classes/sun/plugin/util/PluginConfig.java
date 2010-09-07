/*
 * @(#)PluginConfig.java	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;


/**
 * Plugin Configuration in the Deployment Infrastructure.
 *
 * @author Stanley Man-Kit Ho
 */
final class PluginConfig 
{    
    private static final String JAVAPI = "javapi";
    private static final String CACHE_VERSION = "v1.0";

    /**
     * Construct PluginConfig object.
     */    
    PluginConfig()
    {
	// Make sure all the directories are created
	new File(Config.getUserHome()).mkdirs();
	new File(Config.getSystemHome()).mkdirs();
	new File(getLogDirectory()).mkdirs();
	new File(getSecurityDirectory()).mkdirs();
	new File(getUserExtensionDirectory()).mkdirs();
	new File(new File(getPropertiesFile()).getParent()).mkdirs();
    }

   /**
     * Returns java home in the Deployment Infrastructure.
     */
    public String getJavaHome()
    {
	return Config.getJavaHome();
    }


   /**
     * Returns user home in the Deployment Infrastructure.
     */
    public String getUserHome()
    {
	return Config.getUserHome();
    }


   /**
     * Returns system home in the Deployment Infrastructure.
     */
    public String getSystemHome()
    {
	return Config.getSystemHome();
    }

 
    /**
     * Returns path to the deployment properties file.
     */
    public String getPropertiesFile()
    {
	// ${deployment.user.home}\deployment.properties  (Windows)
	// ${deployment.user.home}/deployment.properties  (Unix)
	//
	return getUserHome() + File.separator + Config.getPropertiesFilename();
    }

  



    /**
     * Returns the ext directory in the Deployment Infrastructure.
     */
    public String getUserExtensionDirectory()
    {
	return Config.getUserExtensionDirectory();
    }


    /**
     * Returns the security directory in the Deployment Infrastructure.
     */
    public String getSecurityDirectory()
    {
	// ${deployment.user.home}\security  (Windows)
	// ${deployment.user.home}/security  (Unix)
	//
	return getUserHome() + File.separator + "security";
    }
    
    
   

    /**
     * Returns the log directory in the Deployment Infrastructure.
     */
    public String getLogDirectory()
    {
	// Check for any user specified path for output files
	//
	String logDir = (String) java.security.AccessController.doPrivileged(
			 new sun.security.action.GetPropertyAction("javaplugin.outputfiles.path"));

	if (logDir == null || logDir.trim().equals(""))
	{
	    // Use default
	    logDir = Config.getLogDirectory();
	}

	return logDir;
    }
}
