/*
 * @(#)JavawsConsoleController14.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.javaws.util;

import java.util.logging.Logger;
import java.util.logging.Level;
import java.security.Policy;

/*
 * JavawsConsoleController14 is a class for controlling 
 * various aspects of the javaws console window behavior.
 *
 * For Java Web Start running with JRE 1.4+ only
 */
public class JavawsConsoleController14 extends JavawsConsoleController
{
    private static Logger logger = null;
    
    public JavawsConsoleController14() {
	super();
    }

    public void setLogger(Logger lg) {
	if (logger == null) {
	    logger = lg;
	}
    }

    public Logger getLogger() {
	return logger;
    }

    /**
     * Return true if security policy reload is supported.
     */
    public boolean isSecurityPolicyReloadSupported() {
	return true;
    }

    /**
     * Reload security policy.
     */
    public void reloadSecurityPolicy() {
	Policy policy = Policy.getPolicy();
	policy.refresh();
    }

      /**
     * Return true if logging is supported.
     */
    public boolean isLoggingSupported() {
	return true;	
    }
     
    /**
     * Toggle logging supported.
     *
     * @return true if logging is enabled.
     */
    public boolean toggleLogging() {
	if (logger != null) {
	    Level level = logger.getLevel();
	
	    if (level == Level.OFF) {
	        level = Level.ALL;
	    } else {
	        level = Level.OFF;
   	    }	
            logger.setLevel(level);

	    return (level == Level.ALL);
	}
	return false;
    }

}
