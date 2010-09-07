/*
 * @(#)ConsoleTraceListener.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.util;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

/**
 * ConsoleTraceListener is a trace listener redirecting output to
 * Java Console.
 */
public final class ConsoleTraceListener implements TraceListener 
{
	private static final long MIN_CONSOLE_OUTPUT_INTERVAL = 100;
	private Object lock = new Object();

    // Console window
    private ConsoleWindow console = null;

    // Buffer to store output while console is not created yet
    //
    private StringBuffer buffer = new StringBuffer();
    
    public ConsoleTraceListener(ConsoleController controller) 
    {
		ConsoleHelper.setConsoleController(controller);
    }

    public void setConsole(ConsoleWindow console)
    {
		this.console = console;
		new ConsoleWriterThread();
    }

    public void print(String msg) 
    {
		synchronized(lock) {
			boolean shouldNotify = (buffer.length() == 0);
			buffer.append(msg);

			if(shouldNotify && console != null)
				lock.notifyAll();
		}
    }

	class ConsoleWriterThread extends Thread {
		public ConsoleWriterThread() {
			super("ConsoleWriterThread");
			setDaemon(true);
			start();
		}

		public void run() {
			long t1 = System.currentTimeMillis();
			long t2;
			while(true) {
				t2 = System.currentTimeMillis();
				synchronized(lock) {
					if(t2 - t1 >= MIN_CONSOLE_OUTPUT_INTERVAL && buffer.length() > 0) {
						console.append(buffer.toString());
						buffer = new StringBuffer();
						t1 = t2;
					} else {
						try {
							if(buffer.length() == 0) {
								lock.wait();
							} else {
								lock.wait(MIN_CONSOLE_OUTPUT_INTERVAL - (t2 - t1));
							}
						} catch (InterruptedException e) {						
						}
					}		
				}			
			}			
		}
	}
}
