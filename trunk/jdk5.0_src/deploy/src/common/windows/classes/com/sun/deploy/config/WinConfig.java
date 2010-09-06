/*
 * @(#)WinConfig.java	1.25 04/03/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.config;

import java.io.File;
import java.io.IOException;
import com.sun.deploy.net.proxy.NSPreferences;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.WinRegistry;

public class WinConfig extends Config {

    static {
	// load native library
	if (Config.isDebugMode()) {
	    System.load(Config.getJavaHome() + File.separator + "bin" + File.separator + "deploy_g.dll");
	} else {
	    System.load(Config.getJavaHome() + File.separator + "bin" + File.separator + "deploy.dll");
	}
    }

    private String _userHome;
    private String _systemHome;
    private String _osHome;
    private String _systemExeHome;

    public String getPlatformUserHome()
    {
	if (_userHome == null)
	{
	    // "user.home" may be overrriden in Java Plug-in.
	    //
	    String profile = (String) java.security.AccessController.doPrivileged(
			       new sun.security.action.GetPropertyAction("javaplugin.user.profile"));

	    if (profile == null || profile.trim().equals(""))
	    {
		// Obtain user path depends on platform
		//
		byte[] buserHome = getPlatformUserHomeImpl();
		
		if (buserHome != null) {
		    // this will take care of the encoding
		    profile = new String(buserHome);
		}
		// Default to {user.home}
		//
		if (profile == null)
		{
		    profile = (String) java.security.AccessController.doPrivileged(
		  		new sun.security.action.GetPropertyAction("user.home"));
    		}
	    }

	    _userHome = profile + File.separator + "Sun" + File.separator + "Java" + File.separator + "Deployment";
	}

	return _userHome;
    }

    public String getPlatformSystemHome()
    {
	if (_systemHome == null)
	    _systemHome = getPlatformSystemHomeImpl() + File.separator + "Sun" + File.separator + "Java" + File.separator + "Deployment";

	return _systemHome;
    }

    public String getPlatformOSHome()
    {
        if (_osHome == null)
        {
            _osHome = getPlatformSystemHomeImpl();
        }
        // no else required; _osHome already set

        return _osHome;
    }

    public String getSystemJavawsPath() {
	if (_systemExeHome == null) {
	    _systemExeHome = getSystemExecutableHomeImpl();
	}

	String path = _systemExeHome + File.separator + "javaws.exe";
	if (!((new File(path)).exists())) {
	    return super.getSystemJavawsPath();
	}
	return path;
    }

    public String getPlatformExtension() {
	return ".exe";
    }

    public String toExecArg(String path) {
	// we need to quote path sent as args to Runtime.exec on windows
	return "\"" + path + "\"";
    }

    public String getLibraryPrefix() {
        return "";	/* no prefix on Windows */
    }

    public String getLibrarySufix() {
        return ".dll";	/* library suffix on Windows */
    }

    public boolean useAltFileSystemView() {
        return true;	/* need AltFileSystemView on windows */
    }

    public boolean systemLookAndFeelDefault() {
	return true;
    }
    
    public String getPlatformSpecificJavaName(){
        return "javaw.exe";
    }

    // we can override this to return session specific string
    public String getSessionSpecificString() {
        return "";
    }

    public String getDebugJavaPath(String path) {
	return path.substring(0, path.lastIndexOf(".")).concat("_g.exe");
    }

    public native boolean showDocument(String url);

    public native String getBrowserPath();

    /**
     * Return Mozilla user profile of the current user
     */
    public String getMozillaUserProfileDirectory()
    {
        /* Get ApplicationData directory since registry.dat is stored there. */
        String appDataDir = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
            "AppData");
        String mozillaUserProfileDir = null;

        if (appDataDir != null)
        {
            try
            {
                File regFile = new File(appDataDir + "\\Mozilla\\registry.dat");

                if (regFile.exists())
                    mozillaUserProfileDir = NSPreferences.getNS6UserProfileDirectory(regFile);
            }
            catch (IOException e)
            {
            }
        }

        return mozillaUserProfileDir;
    }

    public native int installShortcut(String path, String appName,
		String description, String appPath, String args,
		String directory, String iconPath);

    public native int addRemoveProgramsAdd(String jnlpFile, String appName, boolean sysCache);

    public native int addRemoveProgramsRemove(String appName);

    private static native byte[] getPlatformUserHomeImpl();

    private static native String getPlatformSystemHomeImpl();

    private static native String getSystemExecutableHomeImpl();
}
