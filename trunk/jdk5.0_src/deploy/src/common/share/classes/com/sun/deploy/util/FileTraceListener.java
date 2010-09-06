/*
 * @(#)FileTraceListener.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.util;

import java.io.PrintStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.BufferedOutputStream;

public final class FileTraceListener implements TraceListener {

    private File logFile;
    private PrintStream fileTraceStream;

    public FileTraceListener(File logFile, boolean append) {
	this.logFile = logFile;
	init(append);
    }

    private void init(boolean append) {
	try {
	    fileTraceStream = new PrintStream(new BufferedOutputStream(new FileOutputStream(logFile.getPath(), append)));
	} catch (IOException fnfe) {
	    fnfe.printStackTrace();
	}
    }

    public void print(String msg) {	
	fileTraceStream.print(msg);
	fileTraceStream.flush();
    }

}
