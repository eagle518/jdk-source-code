/*
 * @(#)PerfLogger.java	1.5 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import sun.misc.PerformanceLogger;
import java.io.*;
import com.sun.deploy.config.Config;

public final class PerfLogger {

    private static boolean perfLogOn = false;
    private static String perfLogFilePath = null;
    private static long endTime = 0;

    static {
	
	perfLogFilePath = System.getProperty("sun.perflog");
       
	// check if the perfLog filepath is valid
	if (perfLogFilePath != null && Config.isJavaVersionAtLeast15()) {
	    perfLogOn = true;
	}
    }

    public static boolean perfLogEnabled() {
	return perfLogOn;
    }

    public static void setStartTime(String message) {
	if (perfLogOn) {
	    PerformanceLogger.setStartTime(message);
	}
    }

    public static void setTime(String message) {
	if (perfLogOn) {
	    PerformanceLogger.setTime(message);
	}
    }

    public static void setEndTime(String message) {
	if (perfLogOn) {
	    PerformanceLogger.setTime(message);
	    endTime = System.currentTimeMillis();
	}
    }

    public static void outputLog() {
	if (perfLogOn) {
	    if (endTime != 0) {
		PerformanceLogger.setTime("Deployment Java Startup time " + (endTime - PerformanceLogger.getStartTime()));
	    }
	    if (Trace.isEnabled()) {
		PerformanceLogger.outputLog();
	    }
	}

	
    }

   
}
