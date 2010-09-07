/*
 * @(#)PlatformSpecificUtils.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.Vector;
import java.io.File;
import com.sun.deploy.config.Config;

public final class PlatformSpecificUtils{

    public native void init();

    public native void onSave( UpdatePanel info );

    public native void onLoad( UpdatePanel info);

    public native boolean showURL(String url);
    
    /*
     * getPublicJres will return a vector with a list of public jres
     */
    public static native Vector getPublicJres();

    /*
     * getPublicJdks will return a vector with a list of public jdks
     */    
    public static native Vector getPublicJdks();
    
    /*
     * Save APPLET tag usage with browsers.
     */
    public static native void applyBrowserSettings();
    
    /*
     * Get current settings for APPLET tag usage with browsers.
     */
    public static native void initBrowserSettings();

    /*
     * Get the status of Java Quick Starter service.
     */
    public static native boolean getJqsSettings();

    /*
     * Change the status of Java Quick Starter service.
     */
    public static native void setJqsSettings(boolean enable);

    /*
     * Get the current selection of Java Plug-in
     */
    public static native boolean getJavaPluginSettings();

    /*
     * Set the current selection of Java Plug-in
     */
    public static native int setJavaPluginSettings(boolean enable);

    /*
     * Get the long path name representation of the input path.
     */
    public static native String getLongPathName(String path);

    /*
     * Compares 2 paths ignoring case.
     */
    public static boolean samePaths(String path1, String path2) {
	return(path1.equalsIgnoreCase(path2));
    }

    /*
     * Find out if user has Admin privileges 
     */
    public static native boolean getHasAdminPrivileges();
    
    static {
        try {
	    System.load(Config.getJavaHome() + File.separator + "bin" + File.separator + "regutils.dll");
	} catch (Throwable e) {
            //Trace.printException(e);
        }
    }
}
