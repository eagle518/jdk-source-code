/*
 * @(#)UnixConfig.java	1.21 04/03/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.lang.InterruptedException;
import java.lang.Process;
import java.lang.Runtime;
import java.lang.String;
import java.lang.System;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.util.ArrayList;
import com.sun.deploy.net.proxy.NSPreferences;
import com.sun.deploy.util.SearchPath;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public class UnixConfig extends Config {
    static {
	loadLibDeploy();
    }

    public static void loadLibDeploy() {

	String libDir = System.getProperty("os.arch");

	if (libDir.equals("x86"))
	{
	   libDir = "i386";
	}
	
	if (Config.isDebugMode()) {
	    System.load(Config.getJavaHome() + File.separator + "lib" + File.separator + libDir + File.separator + "libdeploy_g.so");
	} else {
	    System.load(Config.getJavaHome() + File.separator + "lib" + File.separator + libDir + File.separator + "libdeploy.so");
	}
    }

    private String _userHome;
    private String _systemHome;
    private String _osHome;

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
		// Default to {user.home}
		//
		profile = (String) java.security.AccessController.doPrivileged(
		  		   new sun.security.action.GetPropertyAction("user.home"));
	    }

	    _userHome = profile + File.separator + ".java" + File.separator + "deployment";
	}

	return _userHome;
    }

    public String getPlatformSystemHome() {
	if (_systemHome == null) {
            _systemHome = File.separator + "etc" +  File.separator + ".java" +  File.separator + "deployment";
	}
	return _systemHome;
    }

    public String getPlatformOSHome() {
	if (_osHome == null) {
	    _osHome = File.separator + "etc";
	}
	return _osHome;
    }

    public String getPlatformExtension() {
	return "";
    }

    public String getLibraryPrefix() {
        return "lib";
    }

    public String getLibrarySufix() {
        return ".so";
    }

    public boolean useAltFileSystemView() {
        return false;
    }

    public boolean isLocalInstallSupported() {
        // Environment variable GNOME_DESKTOP_SESSION_ID is defined
	// for GNOME 2.0 or above
	if (getEnv("GNOME_DESKTOP_SESSION_ID") == null) {
	    return false;
	} else {
	    return true;
	}
    }

    /**   
     * Use the system and feel on Gnome (Gtk), otherwise default (Motif)
     */ 
    public boolean systemLookAndFeelDefault() {
        return (getEnv("GNOME_DESKTOP_SESSION_ID") != null);
    }  

    // we can override this to return a session specific string
    // or a screen specific string (such as the ID of the root X window)
    public String getSessionSpecificString() {
	return "";
    }

    public String getPlatformSpecificJavaName(){
        return "java";
    }

    public static native String getEnv(String name);

    public String getBrowserPath() {
        File          browser = null;
        String     [] list    = null;
        String        result  = null;

        result = getProperty(BROWSER_PATH_KEY);
        if ((result != null) &&
            (result.length() > 0) &&
            (result.equals(getProperty(BROWSER_PATH_DEF)) == false)) {
            // a browser path has been previously set, so make sure it is still
            // valid
            browser = new File(result);

            if (browser.exists() == false) {
                // browser doesn't exist, need to find one
                if (result.equals("netscape")) {
                    // when the path is just "netscape" without any other path
                    // info, it is most likely an old default value, so use the
                    // preferred browser list as it is
                    list = PREFERRED_BROWSERS;

                    // need to search for browser
                    browser = null;
                }
                else {
                    // the path has been set by the user at some point, but it
                    // is no longer valid, so add the browser to the top of the
                    // preferred browser list (this will favor that browser if
                    // it is in the path)
                    list = new String [PREFERRED_BROWSERS.length + 1];

                    list[0] = browser.getName();
                    System.arraycopy(PREFERRED_BROWSERS, 0,
                                     list, 1, PREFERRED_BROWSERS.length);

                    // need to search for browser
                    browser = null;
                }
            }
        }
        else {
            list = PREFERRED_BROWSERS;
        }

        if (browser != null) {
            result = browser.getAbsolutePath();
        }
        else {
            SearchPath search = new SearchPath(getEnv("PATH"));

            browser = search.findOne(PREFERRED_BROWSERS);
            if (browser != null) {
                result = browser.getAbsolutePath();
            }
            else {
                browser = search.findOne(SECONDARY_BROWSERS);
                if (browser != null) {
                    result = browser.getAbsolutePath();
                }
                else {
                    result = BROWSER_PATH_DEF;
                }
            }
        }

        return (result);
    }

    /**
     * Return Mozilla user profile of the current user
     */
    public String getMozillaUserProfileDirectory()
    {
        String mozillaUserProfileDir = null;

        try
        {
            String homeDir = System.getProperty("user.home");
            File regFile = new File(homeDir + "/.mozilla/appreg");

            if (regFile.exists())
                mozillaUserProfileDir = NSPreferences.getNS6UserProfileDirectory(regFile);
        }
        catch (IOException e)
        {
        }

        return mozillaUserProfileDir;
    }

    public boolean showDocument(String url) {
        String  path   = getBrowserPath();
        boolean result = false;

        if (path.equals(getProperty(BROWSER_PATH_DEF)) == false) {
            File browser = new File(path);

            if (browser.exists()) {
                String  [] cmd     = null;
                Process    process = null;

                cmd = getExtendedBrowserArgs(browser, url);
                if (cmd != null) {
                    try {
                        Trace.println("Invoking browser with: \n" +
                                      "     " + argsFromArray(cmd),
                                      TraceLevel.BASIC);

                        process = Runtime.getRuntime().exec(cmd);

                        InputStream    err    = process.getErrorStream();
                        byte        [] buffer = new byte[1024];

                        // wait for exit code -- if it's 0, command worked,
                        // otherwise we need to start the browser up.
                        int exitCode = process.waitFor();
                        int count    = err.read(buffer, 0, 1024);

                        // there should be no error msg if it succeed
                        if ((exitCode == 0) && (count == -1)) {
                            result = true;
                        }
                        // no else required; fall through to using basic args
                    } catch(IOException ioe) {
                        // fall through to using basic args
                        Trace.ignoredException(ioe);
                    } catch(InterruptedException ie) {
                        // fall through to using basic args
                        Trace.ignoredException(ie);
                    }
                }
                // no else required; fall through to using basic args

                if (result == false) {
                    try {
                        cmd = getBasicBrowserArgs(browser, url);

                        Trace.println("Invoking browser with: \n" +
                                      "     " + argsFromArray(cmd),
                                      TraceLevel.BASIC);

                        process = Runtime.getRuntime().exec(cmd);
                        result  = (process != null);
                    } catch(IOException ioe) {
                        // failed to launch browser
                        Trace.ignoredException(ioe);
                    }
                }
                // no else required; already succeeded above
            }
            // no else required; unable to determine a valid browser path
        }
        // no else required; unable to determine a valid browser path

        return (result);
    }

    public static String argsFromArray(String [] args) {
        StringBuffer result = new StringBuffer(ARGS_LIST_CAPACITY * ARG_CAPACITY);

        for (int i = 0; i < args.length; i++) {
            result.append(args[i]).append(SPACE);
        }

        return (result.toString().trim());
    }

    public static String [] argsFromString(String args) {
        ArrayList          list        = new ArrayList(ARGS_LIST_CAPACITY);
        StringBuffer       token       = new StringBuffer(ARG_CAPACITY);
        boolean            matchDouble = false;
        boolean            matchSingle = false;
        int                length      = 0;
        String          [] result      = { };

        if (args != null) {
            length = args.length();
        }

        for (int i = 0; i < length; i++) {
            char ch = args.charAt(i);

            switch (ch) {
                case BACKSLASH:
                    if (++i < length) {
                        // escape sequence
                        token.append(args.charAt(i));
                    }
                    // no else required; nothing to escape
                    break;

                case SINGLE_QUOTE:
                    if (matchDouble) {
                        // add this quote to the current token
                        token.append(ch);
                    }
                    // no else required; this quote isn't part of the argument

                    // toggle single quote matching
                    matchSingle = !matchSingle;
                    break;

                case DOUBLE_QUOTE:
                    if (matchSingle) {
                        // add this quote to the current token
                        token.append(ch);
                    }
                    // no else required; this quote isn't part of the argument

                    // toggle single quote matching
                    matchDouble = !matchDouble;
                    break;

                case TAB:
                case LINEFEED:
                case RETURN:
                case SPACE:
                    if (matchSingle || matchDouble) {
                        // add this character to the current token
                        token.append(ch);
                    }
                    else {
                        // this is a token delimiter
                        if (token.length() > 0) {
                            // add the token to the list
                            list.add(token.toString());
                            token.delete(0, token.length());
                        }
                        // no else required; no token to add
                    }
                    break;

                default:
                    // add this character to the current token
                    token.append(ch);
                    break;
            }
        }

        if (token.length() > 0) {
            // add the last token to the list
            list.add(token.toString());
        }
        // no else required; no token to add

        return ((String []) list.toArray(result));
    }

    private static String [] getBasicBrowserArgs(File browser, String url) {
        String [] result = {
            browser.getAbsolutePath(),
            url
        };

        return (result);
    }

    private static String [] getExtendedBrowserArgs(File browser, String url) {
        boolean    replaced = false;
        String  [] args     = argsFromString(getProperty(EXTENDED_BROWSER_ARGS_KEY));
        String  [] result   = null;

        // substitute %u in any argument for the URL (can't use replaceAll,
        // because JaWS might not be running in a 1.4+ JRE)
        for (int i = 0; i < args.length; i++) {
            int start  = 0;
            int index  = -1;

            index = args[i].indexOf("%u", start);
            while (index >= 0) {
                args[i]  = args[i].substring(start, index) + url + args[i].substring(index + 2);
                start    = index + 2;
                index    = args[i].indexOf("%u", start);

                replaced = true;
            }
        }

        if (replaced) {
            result = new String[args.length + 1];

            System.arraycopy(args, 0, result, 1, args.length);
            result[0] = browser.getAbsolutePath();
        }
        // no else required; invalid extended args

        return (result);
    }

    public String getDebugJavaPath(String path) {
	return path.concat("_g");
    }

    private static String [] PREFERRED_BROWSERS = { "mozilla", "netscape" };
    private static String [] SECONDARY_BROWSERS = { "firebird", "opera", "konqueror", "galeon"};

    private static final int ARGS_LIST_CAPACITY = 32;
    private static final int ARG_CAPACITY       = 64;

    private static final char BACKSLASH    = '\\';
    private static final char SINGLE_QUOTE = '\'';
    private static final char DOUBLE_QUOTE = '\"';
    private static final char TAB          = 0x09;
    private static final char LINEFEED     = 0x0a;
    private static final char RETURN       = 0x0d;
    private static final char SPACE        = 0x20;
}
