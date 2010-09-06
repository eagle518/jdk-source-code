/*
 * @(#)PlatformSpecificUtils.java	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import com.sun.deploy.config.PluginJavaInfo;
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
    
    static {
        try {
	    System.load(Config.getJavaHome() + File.separator + "bin" + File.separator + "RegUtils.dll");
	} catch (Throwable e) {
            //Trace.printException(e);
        }
    }
}
