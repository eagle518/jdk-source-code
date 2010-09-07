/*
 * @(#)Environment.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy;
import java.net.URL;
import java.io.File;
import java.util.Date;
import java.security.AccessController;
import java.security.PrivilegedAction;

public class Environment {
    public static final int ENV_PLUGIN = 0;
    public static final int ENV_JAVAWS = 1;
    public static final int ENV_JCP = 2;
    private static int environmentType = ENV_PLUGIN;
    
    public static final int JAVAFX_INSTALL_ONDEMAND = 0;
    public static final int JAVAFX_INSTALL_PRELOAD_INSTALLER = 1;
    public static final int JAVAFX_INSTALL_AUTOUPDATE = 2;
    private static int javaFxInstallMode = JAVAFX_INSTALL_ONDEMAND;

    private static boolean getenvSupported = true;
    
    private static boolean _javafx_install_initiated = false;

    public static boolean isJavaFXInstallInitiated() {
        return _javafx_install_initiated;
    }

    public static void setDownloadInitiated(boolean initiated) {
        // for javafx preload or auto-update
        if (getJavaFxInstallMode() !=
                Environment.JAVAFX_INSTALL_ONDEMAND) {
            _javafx_install_initiated = initiated;
        }
    }

    public static boolean allowAltJavaFxRuntimeURL() {

        try {
            if (getenvSupported &&
                    System.getenv("ALLOW_ALT_JAVAFX_RT_URL") != null) {
                return true;
            }
        } catch (Error e) {
            getenvSupported = false;
            // System.getenv not supported
        }

        return false;
    }

    public static int getJavaFxInstallMode() {
        return javaFxInstallMode;
    }

    public static void setJavaFxInstallMode(int mode) {
        if (isImportMode() == false && (
                mode == JAVAFX_INSTALL_PRELOAD_INSTALLER ||
                mode == JAVAFX_INSTALL_AUTOUPDATE)) {
            // can only set preload or auto-update mode if we are doing
            // import
            return;
        }
        javaFxInstallMode = mode;
    }

    // javaws and jcp have one entry point and will set the environmentType
    // plugin has multiple entry points so it will default to plugin type
    public static void setEnvironmentType(int type) {
        environmentType = type;
    }

    public static void reset() {
    }

    public static boolean isJavaWebStart() {
        return (environmentType == ENV_JAVAWS);
    }

    public static boolean isJavaControlPanel() {
        return (environmentType == ENV_JCP);
    }

    public static boolean isJavaPlugin() {
        return (environmentType == ENV_PLUGIN);
    }

    private static String codebaseOverride = null;
    private static URL codebase = null;
    private static Date timestamp = null;
    private static Date expiration = null;
    
    public static void setImportModeTimestamp(Date d) {
        timestamp = d;
    }
    
    public static Date getImportModeTimestamp() {
        return timestamp;
    }
    
    public static void setImportModeExpiration(Date d) {
        expiration = d;
    }
    
    public static Date getImportModeExpiration() {
        return expiration;
    }

    // set the import codebase - this is the codebase specified in the jnlp file
    public static void setImportModeCodebase(URL u) {
        codebase = u;
    }
    
    // set the import codebase override - this is from user command line input
    public static void setImportModeCodebaseOverride(String s) {
        // this string must be a url, so make sure it ends with /
        if (s != null && !s.endsWith("/")) {
            s = s + "/";
        }
        codebaseOverride = s;
    }
    
    // get the import mode codebase URL
    public static URL getImportModeCodebase() {
        return codebase;
    }
    
    // get the import mode codebase override value
    public static String getImportModeCodebaseOverride() {
        return codebaseOverride;
    }
    
    private static String userAgent = null;
    
    // set the http header user agent field
    public static void setUserAgent(String ua) {
        userAgent = ua;
    }

    // get the http header user agent field
    public static String getUserAgent() {
	return userAgent;
    }
    
    private static boolean isSystemCache = false;
    
    // returns true if we are in system cache mode
    public static boolean isSystemCacheMode() { 
        return isSystemCache; 
    }
    
    // set system cache mode
    public static void setSystemCacheMode(boolean s) { 
        isSystemCache = s; 
    }
    
    private static boolean isSilentMode = false;
    
    // returns true if we are in silent mode
    public static boolean isSilentMode() { 
        return isSilentMode; 
    }
    
    // set silent mode
    public static void setSilentMode(boolean s) { 
        isSilentMode = s; 
    }
    
    private static boolean isImportMode = false;
    
    // returns true if we are in import mode
    public static boolean isImportMode() { 
        return isImportMode; 
    }
    
    // set import mode
    public static void setImportMode(boolean s) { 
        isImportMode = s; 
    }
    
    private static boolean isInstallMode = false;
    
    // returns true if we are in install mode
    public static boolean isInstallMode() { 
        return isInstallMode; 
    }
    
    // set install mode when we are running extension installer or during
    // uninstall of applications
    public static void setInstallMode(boolean s) { 
        isInstallMode = s; 
    }
}
