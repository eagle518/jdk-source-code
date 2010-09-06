/*
 * @(#)LoggerTraceListener.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.File;
import java.util.logging.FileHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.io.IOException;

public final class LoggerTraceListener implements TraceListener {

    private Logger logger;

    public LoggerTraceListener(String loggerName, String filename) {
	Handler handler = null;

	// Create logger object
	logger = Logger.getLogger(loggerName);
	
	try {
	    handler = new FileHandler(filename);
	    // Send logger output to our FileHandler
	    logger.addHandler(handler);
	
	    // default level is OFF
	    logger.setLevel(Level.OFF);
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
	    
    }

    public Logger getLogger() {
	return logger;
    }

    public void print(String msg) {	
	logger.log(Level.FINE, msg);
    }

}
