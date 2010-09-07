/*
 * @(#)PlatformSpecificUtils.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.Vector;
import java.util.StringTokenizer;
import java.io.File;

public final class PlatformSpecificUtils{

    public void init(){}

    public void onSave( UpdatePanel info ){}

    public void onLoad( UpdatePanel info){
        
    }
    
    public static Vector getPublicJres(){
	String home = System.getProperty("user.home");
	// new algorithm for determining the installed JRE's: find out where
	// the javaplugin.jar came from (that's where the plugin was installed
	// and then user's home directory.

	// Modified to search for rt.jar instead of javaplugin.jar, because
	// the plugin and the JRE can be separated, as in Solaris 8.

	String classpath = System.getProperty("java.class.path");
	String bootclasspath = System.getProperty("sun.boot.class.path");

	if (classpath == null)
	    classpath = bootclasspath;
	else
	    classpath = classpath + File.pathSeparator + bootclasspath;

	StringTokenizer parser = new StringTokenizer(classpath,
						     File.pathSeparator);
	String path = "";
	while (parser.hasMoreElements()) {
	    path = (String) parser.nextElement();
	    if (path.endsWith("rt.jar"))
		break;
	}
	int index = path.lastIndexOf(File.separatorChar);
	path = path.substring(0, index);
	index = path.lastIndexOf(File.separatorChar);
	path = path.substring(0, index);
	
	String version = System.getProperty("java.version");
	int versionIdx = version.lastIndexOf("-");

	Vector vector = new Vector();
	if (versionIdx != -1) {
	   version = version.substring(0, versionIdx);
	}
	vector.addElement(version);
	vector.addElement(path);        
        
        return vector;
    }
    
    public static Vector getPublicJdks(){
        return new Vector();
    }
    
    /*
     * These are not needed on unix.  Dummies.
     */
    public static void applyBrowserSettings(){}
    public static void initBrowserSettings(){}
    public static boolean getJqsSettings(){
        // JQS is not running on unix.
        return false;
    }
    public static void setJqsSettings(boolean enable){}

    public static boolean getJavaPluginSettings(){
	// Java Plug-in selection not available on unix
	return false;
    }

    public static int setJavaPluginSettings(boolean enable){
	// Java Plug-in selection not available on unix
	// return 2 so that ControlPanel won't popup a success dialog
	return 2;
    }

    public static String getLongPathName(String path) {
	// On unix platforms, just return the input path.
	return path;
    }

    /*
     * Compares 2 paths.
     */
    public static boolean samePaths(String path1, String path2) {
	return(path1.equals(path2));
    }

    public static boolean getHasAdminPrivileges(){
        return false;
    }


    public boolean showURL(String url){ return false; }    


}
