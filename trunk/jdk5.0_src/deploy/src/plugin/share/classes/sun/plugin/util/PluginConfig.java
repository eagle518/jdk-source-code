/*
 * @(#)PluginConfig.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.io.File;
import com.sun.deploy.config.Config;


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
	new File(getCacheDirectory()).mkdirs();
	new File(getPluginCacheDirectory()).mkdirs();
	new File(getTempDirectory()).mkdirs();
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
     * Returns the temp directory in the Deployment Infrastructure.
     */
    public String getTempDirectory()
    {
	// ${deployment.user.cachedir}\tmp  (Windows)
	// ${deployment.user.cachedir}/tmp  (Unix)
	//
	return getCacheDirectory() + File.separator + "tmp";
    }


    /**
     * Returns the cache directory in the Deployment Infrastructure.
     */
    public String getCacheDirectory()
    {
	return Config.getCacheDirectory();
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
     * Returns the directory structure under the top of the cache directory
     */
    private String getCacheDirectorySubStructure()
    {
        return JAVAPI + File.separator + CACHE_VERSION;
    }
    
    /**
     * Returns the cache directory in the Deployment Infrastructure.
     */
    public String getPluginCacheDirectory()
    {
        // Check for the user-specified cache directory
        String cacheDir = Config.getCacheDirectory() + File.separator + getCacheDirectorySubStructure();

        return cacheDir;
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

    public String getTraceFile() {
	return Config.getProperty(Config.JPI_TRACE_FILE_KEY);
    }

    public String getLogFile() {
	return Config.getProperty(Config.JPI_LOG_FILE_KEY);
    }
}
