/*
 * @(#)WinConfig.java	1.56 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.config;

import java.io.*;

import java.io.File;
import java.io.IOException;
import java.util.Vector;
import com.sun.deploy.net.proxy.NSPreferences;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.WinRegistry;
import com.sun.deploy.association.utility.WinRegistryWrapper;

public class WinConfig extends Config {

    static {
	Config.getInstance().loadDeployNativeLib();
    }

    private String _userHome;
    private String _systemHome;
    private String _osHome;
    private String _systemExeHome;
    
    public void loadDeployNativeLib() {
        // load native library
        try {
	    // Only load msvcr71.dll for 32bit JRE
   	    if (!System.getProperty("os.arch").equalsIgnoreCase("amd64")) {
                // load msvcr71.dll first
                System.load(Config.getJavaHome() + File.separator + "bin" +
                    File.separator + "msvcr71.dll");
	    }

            // then load deploy.dll
            System.load(Config.getJavaHome() + File.separator + "bin" + 
                    File.separator + "deploy.dll");
        } catch (UnsatisfiedLinkError e) {
            // should not happen
        }
    }
    
    public String escapeBackslashAndQuoteString(String in) {
        if (in == null) return in;
        
        StringBuffer e = new StringBuffer();
        e.append("\"");
        for (int i = 0; i < in.length(); i++) {
            char a = in.charAt(i);
            if (a == '\\') {
                e.append('\\');
            }
            e.append(a);
        }
        e.append("\"");
        return e.toString();
    }

    private static final String JRE_SUBKEY="SOFTWARE\\JavaSoft\\Java Runtime Environment";
    private static final String JAVA_HOME="JavaHome";

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

	    _userHome = profile + (profile.endsWith(File.separator) ? "" : File.separator) + 
                    "Sun" + File.separator + "Java" + File.separator + "Deployment";

	}

	return _userHome;
    }

    public String getDefaultSystemCache() {
        // default system cache is a separate cache in the user's directory
        // currently only used for javaws -import -system during JRE install
        // preload or auto-update
        return getPlatformUserHome() + File.separator + "SystemCache";
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
    
    private static String replace(final String input, final String oldPattern,
        final String newPattern){
        
        final StringBuffer result = new StringBuffer();
        //startIdx and idxOld delimit various chunks of input; these
        //chunks always end where oldPattern begins
        int startIdx = 0;
        int idxOld = 0;
        while ((idxOld = input.indexOf(oldPattern, startIdx)) >= 0) {
            //grab a part of input which does not include oldPattern
            result.append( input.substring(startIdx, idxOld) );
            //add newPattern to take place of oldPattern
            result.append( newPattern );
            
            //reset the startIdx to just after the current match, to see
            //if there are any further matches
            startIdx = idxOld + oldPattern.length();
        }
        //the final chunk will go to the end of input
        result.append( input.substring(startIdx) );
        return result.toString();
    }


    public String getSystemJavawsPath() {
	if (_systemExeHome == null) {
	    _systemExeHome = getSystemExecutableHomeImpl();
	}
	String path = _systemExeHome + File.separator + "javaws.exe";
	File execFile = new File(path);
	if (execFile.exists()) {
            // The following is temporary, till we have a 64 bit deploymnent.
            // If you are in a 32 bit process, the %windir%\system32\javaws.exe
            // will appear to exist,  only because %windir%\system32 is mapped 
            // to %windir%\SysWow64 in a 32 bit process on a 64 bit windows OS.
	    File execFile32 = 
		new File(replace(path, "system32", "SysWOW64"));
	    if (execFile32.exists()) {
		return execFile32.getPath();
	    }
	    return execFile.getPath();
	} else {
	    return super.getSystemJavawsPath();
	}
    }

    public long getSysStartupTime() {
        return (System.currentTimeMillis() - getSysTickCount());
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
	return "-" + System.getProperty("os.arch");
    }

    public String getDebugJavaPath(String path) {
	return path.substring(0, path.lastIndexOf(".")).concat(".exe");
    }

    public void resetJavaHome() {
        String version = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE,
            "Software\\Javasoft\\Java Runtime Environment", "CurrentVersion");
	if (version != null) {
	    String path = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE,
		"Software\\Javasoft\\Java Runtime Environment\\" + version,
		"JavaHome");
	    if (path != null) {
		Trace.println("_javaHome reset from: " + _javaHome + 
			      " to: " + path, TraceLevel.NETWORK);
		_javaHome = path;
	    }
	}
    }

    public native long getSysTickCount();

    public native boolean showDocument(String url);

    public native String getBrowserPath();

    public native void notifyJREInstalled(String jreBinPath);

    public native boolean isNativeModalDialogUp();

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

    /**
     * Return FireFox user profile of the current user
     */
    public String getFireFoxUserProfileDirectory()
    {
        /* Get ApplicationData directory since registry.dat is stored there. */
        String appDataDir = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
            "AppData");
        String fireFoxUserProfileDir = null;

        if (appDataDir != null)
        {
            try
            {
	        // This is for new FireFox release
                File iniFile = new File(appDataDir + "\\Mozilla\\Firefox\\profiles.ini");
                if (iniFile.exists())
                    fireFoxUserProfileDir = NSPreferences.getFireFoxUserProfileDirectory(iniFile);
            }
            catch (IOException e)
            {
            }
        }

        return fireFoxUserProfileDir;
    }
    
    private static void execute(String[] commands, int msTimeout) {
        Process p = null;
        boolean done = false;

        try {
            p = Runtime.getRuntime().exec(commands);
        } catch (IOException ioe) {
            done = true;
        }
        int exitValue = -1;
        int waitCount = msTimeout / 100;
        while (!done) {
            // Wait a bit.
            try {
                Thread.sleep(100);
            } catch (InterruptedException ite) {
            }

            // Check if done.
            try {
                exitValue = p.exitValue();
                done = true;
            } catch (IllegalThreadStateException itse) {
                if (--waitCount == 0) {
                    // Give up on it!
                    done = true;
                    p.destroy();
                }
            }
        }
    }

    public boolean canAutoDownloadJRE() {
        // allow auto download on Vista same as XP:
        // if (isPlatformWindowsVista()) {
        //     return false;
        // }
        return canDownloadJRE();
    }

    public int getSystemShortcutIconSize(boolean isDesktop) {
        if (isDesktop) {
            int size = getDesktopIconSize();
            if (size >=16 && size <=64) {
                return size;
            }
            return ((isPlatformWindowsVista()) ? 48 : 32);
        } else {
            return 16;
        }
    }

    /**Get public installed JRE from windows registry
     * This is similar to getPublicJres() in windows' com.sun.deploy.panel.PlatformSpecificUtils.java
     * We don't require a minimal version here. All installed versions will be returned.
     * Plugin2 JVM Manager will filter out unusable versions.
     */
    public Vector getInstalledJREList() {
	// The vector contains jre version and jre home path pairs
        Vector/*<String>*/ jreList = new Vector();	
        int subKeyIndex = 0;
        String subKeyName, javaHomePath;
	
        while ( (subKeyName = WinRegistryWrapper.WinRegEnumKeyEx(WinRegistryWrapper.HKEY_LOCAL_MACHINE, 
								 JRE_SUBKEY, subKeyIndex, 
								 WinRegistryWrapper.MAX_KEY_LENGTH)) != null) {
	    jreList.add(subKeyName);	    
	    if ((javaHomePath = WinRegistryWrapper.WinRegQueryValueEx(WinRegistryWrapper.HKEY_LOCAL_MACHINE, 
								      JRE_SUBKEY+"\\"+subKeyName, JAVA_HOME)) != null) {
		jreList.add(javaHomePath);
	    } else {
		// add an empty string to pad the jreList
		jreList.add("");
	    }	    
	    subKeyIndex++;
        }
        return jreList;
    }
    
    /** Return the path to the running Mozilla Browser */
    public String getBrowserHomePath() {
	return getBrowserHomePathImpl();
    }

    public void sendJFXPing(String installMethod, String pingName,
            String currentJavaFXVersion, String requestJavaFXVersion,
            String currentJavaVersion, int returnCode, String errorFile) {
   
        String regUtilsPath = Config.getJavaHome() + File.separator + "bin" +
                    File.separator + "regutils.dll";
        sendJFXPingImpl(regUtilsPath, installMethod, pingName, 
                currentJavaFXVersion, requestJavaFXVersion, currentJavaVersion,
                returnCode, errorFile);
    }

    /** Returns the setting of the USERPROFILE env. variable
     */
    public String getUserHomeOverride() {
        return System.getenv("USERPROFILE");
    }

    /** Sets the user.home system property
     */
    public void setUserHomeOverride(String userHome) {
        java.util.Properties systemProps = System.getProperties();
        systemProps.put("user.home", userHome); 
    }

    private native void sendJFXPingImpl(String regUtilsPath, String installMethod, String pingName,
            String currentJavaFXVersion, String requestJavaFXVersion,
            String currentJavaVersion, int returnCode, String errorFile);
    
    public native int installShortcut(String path, String appName,
		String description, String appPath, String args,
		String directory, String iconPath);

    /**
     * Register a JavaWS application's Uninstall key in the Windows registry so
     * that it is listed in the Add and Remove Programs control panel.
     *
     * @param jnlpURL     the URL used to launch the JNLP program.  (required)
     * @param title       the name (ex. the <title> tag) to display in the
     *                    add/remove applet list.  (required)
     * @param icon        the path to the program icon in an appropriate system
     *                    format.  If null, then a default icon is used.
     * @param vendor      the program vendor if specified in the JNLP file;
     *                    otherwise null.
     * @param description the program description if specified in the JNLP file;
     *                    otherwise null.
     * @param homepage    the program homepage if specified in the JNLP file;
     *                    otherwise null.
     * @param sysCache    true if the program is installed in the system cache;
     *                    false otherwise.
     */
    public native void addRemoveProgramsAdd(String  jnlpURL,
                                            String  title,
                                            String  icon,
                                            String  vendor,
                                            String  description,
                                            String  homepage,
                                            boolean sysCache);

    /**
     * Remove a JavaWS application's Uninstall key in the Windows registry so
     * that it is no longer listed in the Add and Remove Programs control panel.
     *
     * @param title       the name (ex. the <title> tag) to display in the
     *                    add/remove applet list.  (required)
     */
    public native void addRemoveProgramsRemove(String title,boolean sysCache);

    public native boolean canDownloadJRE();

    public native boolean isPlatformWindowsVista();

    public native boolean isBrowserFireFox();

    private static native byte[] getPlatformUserHomeImpl();

    private static native String getPlatformSystemHomeImpl();

    private static native String getSystemExecutableHomeImpl();

    private static native String getBrowserHomePathImpl();

    /*
     * Windows:
     *    Windows XP or later: 8191 characters
     *    Windows 2000 or Windows NT 4.0: 2047 character
     *    Common size: 2047
     */
    protected native int getPlatformMaxCommandLineLength();

    protected native long getPlatformPID();
 
    protected native int getDesktopIconSize();
}
