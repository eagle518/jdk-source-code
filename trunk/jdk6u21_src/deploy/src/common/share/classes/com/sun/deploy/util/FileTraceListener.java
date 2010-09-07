/*
 * @(#)FileTraceListener.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.util;

import java.io.PrintStream;
import java.io.File;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import com.sun.deploy.config.Config;

public final class FileTraceListener implements TraceListener {

    private File logFile;
    private PrintStream fileTraceStream;
    private boolean append;

    public FileTraceListener(File logFile, boolean append) {
	this.append = append;
	this.logFile = logFile;
	init();
    }

    private void init() {
	try {
	    fileTraceStream = new PrintStream(new BufferedOutputStream(new FileOutputStream(logFile.getPath(), append)));
	} catch (IOException fnfe) {
	    fnfe.printStackTrace();
	}
    }

    public void print(String msg) {	
	try{
	    //If the trace file is bigger than the max, cut the size in half
	    if (logFile.length() >= (Config.getIntProperty(Config.MAX_SIZE_FILE_KEY) * 1048576 )) {
		
		fileTraceStream.close();

		File tempTraceFile = File.createTempFile("javaws", ".temp", logFile.getParentFile());
		long size = logFile.length() / 4 ;
		
		BufferedReader in = new BufferedReader(new FileReader(logFile));
		BufferedWriter out = new BufferedWriter(new FileWriter(tempTraceFile));
		
		in.skip(size * 3);
		int c;
		
		while ((c = in.read()) != -1)
		    out.write(c);
		
		in.close();
		out.close();
		
		if(logFile.delete()){
		    tempTraceFile.renameTo(logFile);
		    init();
		}
	    }	
	    fileTraceStream.print(msg);
	    fileTraceStream.flush();
	
	} catch (IOException fnfe) {
	    fnfe.printStackTrace();
	}
    }
}
