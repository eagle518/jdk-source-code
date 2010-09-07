/*
 * @(#)LoggerTraceListener.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.File;
import java.util.logging.FileHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.io.IOException;
import com.sun.deploy.config.Config;

public final class LoggerTraceListener implements TraceListener {

    private Logger logger;

    public LoggerTraceListener(String loggerName, String filename) {
	Handler handler = null;

	// Create logger object
	logger = Logger.getLogger(loggerName);
        // Prevent this logger from sending output to its parent handlers
        // to prevent infinite loops when logging and tracing are enabled
        // and java.util.logging.ConsoleHandler.level is below a certain level
        logger.setUseParentHandlers(false);
	
	try {
	    handler = new FileHandler(filename, (Config.getIntProperty(Config.MAX_SIZE_FILE_KEY) * 1048576 ), 1);
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
