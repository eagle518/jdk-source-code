/*
 * @(#)PlatformSpecificUtils.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

//import com.sun.deploy.config.Config;
import com.sun.deploy.config.PluginJavaInfo;
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
	
	Vector vector = new Vector();
	vector.addElement(System.getProperty("java.version"));
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


    public boolean showURL(String url){ return false; }    


}
