/*
 * @(#)SecureStaticVersioning.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.lang.String;
import java.io.IOException;
import java.awt.Component;
import java.net.URL;

import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JREDesc;
import com.sun.javaws.Globals;

import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.VersionID;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.SecurityBaseline;

public class SecureStaticVersioning extends Object {

    private final static String SSVERSION_KEY = "ssv.version.allowed";
    private static String acceptedVersion = null;

    /**
     * Display a dialog to determine if the user wants to allow Java Web Start
     * to download a particular JRE version.
     *
     * @param parent  - component to parent the dialog.
     * @param ld      - the application launch descriptor.
     *
     * @return <code>true</code> if the user wants to allow the download 
     *         <code>false</code> otherwise.
     */
    public static boolean promptDownload(Component parent, LaunchDesc ld,
	LocalApplicationProperties lap, String version, String source) {

        String title = ResourceManager.getString("javaws.ssv.title");

        String message = 
	    ResourceManager.getString("javaws.ssv.download.masthead");
	
        String bullet = ResourceManager.getString(
 	    "javaws.ssv.download.bullet", version, source);

	String download = 
	    ResourceManager.getString("javaws.ssv.download.button");
	String cancel = ResourceManager.getString("common.cancel_btn");

        int result = UIFactory.showConfirmDialog(parent, ld.getAppInfo(),
		     message, bullet, title, download, cancel, true);
	if (result == UIFactory.OK) {
	    acceptedVersion = version;
	    if (lap != null) {
                lap.put(SSVERSION_KEY, version);
	        try { 
		    lap.store(); 
	        } catch(IOException ioe) { 
		    Trace.ignoredException(ioe); 
	        }
            }
	    return true;
	}
        return false;
    }

    public static boolean promptUse(Component parent, LaunchDesc ld,
	LocalApplicationProperties lap, JREInfo info) {


	String version = info.getProduct();
	String source = "";

	String href = info.getLocation();
	if (href == null) {
	    href = Config.getProperty(Config.JAVAWS_JRE_INSTALL_KEY);
	}
	try {
	    source = (new URL(href)).getHost();
	} catch (Throwable t) {
	    Trace.ignored(t);
	}
	
        String title = ResourceManager.getString("javaws.ssv.title");

        String message = ResourceManager.getString(
				"javaws.ssv.runold.masthead");

        String bullet = ResourceManager.getString(
				"javaws.ssv.runold.bullet", version, source);

	String run = ResourceManager.getString("javaws.ssv.run.button");
	String cancel = ResourceManager.getString("common.cancel_btn");

	int result = UIFactory.showConfirmDialog(parent, ld.getAppInfo(),
			   message, bullet, title, run, cancel, true);
	if (result == UIFactory.OK) {
	    acceptedVersion = version;
            if (lap != null) {
	        lap.put(SSVERSION_KEY, version);
	        try { 
		    lap.store(); 
	        } catch(IOException ioe) { 
		    Trace.ignoredException(ioe); 
	        }
            }
	    return true;
	}
        return false;
    }


    public static boolean promptRequired(LaunchDesc ld, 
	LocalApplicationProperties lap, boolean forDownload, String version) {
	String lapVersion = (lap == null) ? null : lap.get(SSVERSION_KEY);
	
	if (!(SecurityBaseline.satisfiesSecurityBaseline(version)) &&
            (isOlderVersion(version)) &&
	    (Config.getBooleanProperty(Config.JAVAWS_SSV_ENABLED_KEY)) &&
	    (ld.getSecurityModel() == ld.SANDBOX_SECURITY) &&
	    !(version.equals(acceptedVersion)) &&
	    !(version.equals(lapVersion))) {
	    return true;
	}
	return false;
    }

    private static boolean isOlderVersion(String version) {
	// compare requested version to see if equal or greater than this
	VersionID thisPlus = new VersionID(Globals.getJavawsVersion() + "+");
	VersionID requested = new VersionID(version);
	return !thisPlus.match(requested);
    }

}
