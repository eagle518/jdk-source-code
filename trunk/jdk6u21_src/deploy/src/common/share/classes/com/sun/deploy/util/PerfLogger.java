/*
 * @(#)PerfLogger.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.*;
import sun.misc.PerformanceLogger;
import com.sun.deploy.config.Config;

public final class PerfLogger {

    private static boolean perfLogOn = false;
    private static String perfLogFilePath = null;
    private static long baseTime = 0;
    private static long endTime = 0;

    static {
	
	perfLogFilePath = System.getProperty("sun.perflog");
       
	// check if the perfLog filepath is valid
	if (perfLogFilePath != null && Config.isJavaVersionAtLeast15()) {
	    perfLogOn = true;
	}
    }

    public static void setBaseTimeString(String s) {
        if (s != null) {
            try {
                long time = (new Long(s)).longValue();
                baseTime = time + Config.getInstance().getSysStartupTime();
            } catch (Exception e) {
            }
        }
    }

    public static boolean perfLogEnabled() {
	return perfLogOn;
    }

    public static void setStartTime(String message) {
	if (perfLogOn) {
            if (baseTime == 0) {
                baseTime = System.currentTimeMillis();
            }
	    PerformanceLogger.setStartTime(message, baseTime);
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
        try {
            PerformanceLogger.setBaseTime(baseTime);
        } catch (NoSuchMethodError e) {
            // ignore error i jre is before this method existed
        }
	if (perfLogOn) {
	    if (endTime != 0) {
		PerformanceLogger.setTime("Deployment Java Startup time " + (endTime - PerformanceLogger.getStartTime()));
	    }
	    PerformanceLogger.outputLog();
	}

	
    }

   
}
