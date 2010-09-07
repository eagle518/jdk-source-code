/*
 * @(#)Trace.java	1.32 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.awt.Component;
import java.io.PrintStream;
import java.io.File;
import java.io.FileFilter;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Arrays;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;
import java.util.StringTokenizer;
import com.sun.deploy.ui.UIFactory;


public class Trace {
    private static boolean _bAutomationOn = false;

    // message queue
    private static final ArrayList queue = new ArrayList();

    // list of registered listeners
    private static final ArrayList listenersList = new ArrayList();

    // list of enabled TraceLevel
    private static HashSet traceLevelList = new HashSet();

    // TracePrintStream is used to redirect System.out and System.err
    private static PrintStream psOut = new PrintStream(new TracePrintStream());
    private static PrintStream psErr = new PrintStream(new TracePrintStream());
    
    private Trace() {}

    static {
	// DEFAULT TraceLevel enabled by default
	traceLevelList.add(TraceLevel.DEFAULT);

	Thread traceMsgQueueThread = new Thread(new TraceMsgQueueChecker(), "traceMsgQueueThread");

	traceMsgQueueThread.setDaemon(true);

	// start the thread to monitor the message queue
	traceMsgQueueThread.start();
    }

    public static void redirectStdioStderr()
    {
	// redirect System.out and System.err	
	System.setOut(psOut);
	System.setErr(psErr);
    }

    private static class TraceMsgQueueChecker implements Runnable {

        public void run() {

	    // give initialization a few seconds to add some trace listeners ...
	    try {
		Thread.sleep(2000);
	    } catch (InterruptedException ie) {}

	    // Infinite loop
	    while (true) {
	        TraceMessage tm = null;
	        // check if there are any messages in the queue
	        synchronized(queue) {
		    if (queue.isEmpty()) {
		        try {
			    // queue is empty
			    queue.wait();
		        } catch (InterruptedException ie) {
			    ie.printStackTrace();
		        }		    
		    } else {
		        // Remove TraceMessage from queue
		        tm = (TraceMessage)queue.remove(0);
		    }
	        }
	        // send to registered listeners
	        // Notice that this operation is OUTSIDE the
	        // synchronization block. This is to reduce the
	        // chance to block System.out and System.err
	        if (tm != null) {
		    firePrintlnEvent(tm);
	        }
	    }
        }
    }

    public static void flush() {
        while (true) {
            synchronized(queue) {
                if (queue.isEmpty()) {
                    return;
                }
            }
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) { }
        }
    }

    /**  
     * Determine if tracing is enabled.
     *   
     * @return true if tracing is enabled
     */  
    public static boolean isEnabled()
    {
	// traveLevelList size is at least 1 because TraceLevel.DEFAULT is
	// always in there
	return (traceLevelList.size() > 1);
    }   

    public static void setInitialTraceLevel() {
	String initTraceLevel = Config.getProperty(Config.TRACE_LEVEL_KEY);
	setInitialTraceLevel(initTraceLevel);
    }
    
    public static void setInitialTraceLevel(String initTraceLevel) {
	if (initTraceLevel != null && initTraceLevel.equals("") == false) {
	    StringTokenizer st = new StringTokenizer(initTraceLevel, "|");
     
	    while (st.hasMoreTokens()) {
		String option = (String) st.nextToken();
		
		if (option.equalsIgnoreCase("all")) {
		    setBasicTrace(true);
		    setCacheTrace(true);
		    setNetTrace(true);
		    setTempTrace(true);
		    setSecurityTrace(true);
		    setExtTrace(true);
		    setLiveConnectTrace(true);
		    
		    // If "all", then there is no need to check the rest
		    break;
		} else if (option.equalsIgnoreCase("basic")) {
		    setBasicTrace(true);
		} else if (option.equalsIgnoreCase("cache")) {
		    setCacheTrace(true);
		} else if (option.equalsIgnoreCase("net")) {
		    setNetTrace(true);
		} else if (option.equalsIgnoreCase("temp")) {
		    setTempTrace(true);
		} else if (option.equalsIgnoreCase("security")) {
		    setSecurityTrace(true);
		} else if (option.equalsIgnoreCase("ext")) {
		    setExtTrace(true);
		} else if (option.equalsIgnoreCase("liveconnect")) {
		    setLiveConnectTrace(true);
		}
	    }
	}
    }
   
    /**
     * Turn on/off basic tracing.
     *   
     * @param true if basic tracing should be turned on.
     */  
    public static void setBasicTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.BASIC);
	} else { 
	    traceLevelList.remove(TraceLevel.BASIC);
	}
    }


    /**
     * Turn on/off network tracing.
     *   
     * @param true if network tracing should be turned on.
     */  
    public static void setNetTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.NETWORK);
	} else { 
	    traceLevelList.remove(TraceLevel.NETWORK);
	}
    }

    /**
     * Turn on/off cache tracing.
     *   
     * @param true if cache tracing should be turned on.
     */  
    public static void setCacheTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.CACHE);
	} else { 
	    traceLevelList.remove(TraceLevel.CACHE);
	}
    }

    /**
     * Turn on/off temp tracing.
     *   
     * @param true if temp tracing should be turned on.
     */  
    public static void setTempTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.TEMP);
	} else { 
	    traceLevelList.remove(TraceLevel.TEMP);
	}
    }

    /**
     * Turn on/off security tracing.
     *   
     * @param true if security tracing should be turned on.
     */  
    public static void setSecurityTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.SECURITY);
	} else { 
	    traceLevelList.remove(TraceLevel.SECURITY);
	}
    }


    /**
     * Turn on/off extension tracing.
     *   
     * @param true if extension tracing should be turned on.
     */  
    public static void setExtTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.EXTENSIONS);
	} else { 
	    traceLevelList.remove(TraceLevel.EXTENSIONS);
	}
    }


    /**
     * Turn on/off LiveConnect tracing.
     *   
     * @param true if liveconnect tracing should be turned on.
     */  
    public static void setLiveConnectTrace(boolean trace)
    {
	if (trace) {
            traceLevelList.add(TraceLevel.LIVECONNECT);
	} else { 
	    traceLevelList.remove(TraceLevel.LIVECONNECT);
	}
    }

    /**
     * Determine if automation is enabled.
     *   
     * @return true if automation is enabled
     */  
    public static boolean isAutomationEnabled()
    {
        return _bAutomationOn;
    }   

    /**
     * Enable/disable automation.
     */  
    public static void enableAutomation(boolean bAutomation)
    {
        _bAutomationOn = bAutomation;
    }   

    // add the msg to the queue
    private static void enQueue(TraceMessage tm) {
	synchronized(queue) {
	    // Add element to queue
	    queue.add(tm);
	    
	    // Notify Trace thread
	    queue.notifyAll();
	}	  	
    }

    public static boolean isTraceLevelEnabled(TraceLevel level) {
        return traceLevelList.contains(level);
    }

    public static void print(String message, TraceLevel traceLevel) {
	if (traceLevelList.contains(traceLevel)) {
	    TraceMessage tm = new TraceMessage(traceLevel, message);
	    // post message to the queue
	    enQueue(tm);
	}
    }
    

    public static void println(String message, TraceLevel traceLevel) {
	if (traceLevelList.contains(traceLevel)) {
	    TraceMessage tm = new TraceMessage(traceLevel, message, true);
	    // post message to the queue
	    enQueue(tm);
	}
    }

    public static void println(String message) {
	println(message, TraceLevel.DEFAULT);
    }

    public static void print(String message) {
	print(message, TraceLevel.DEFAULT);
    }

    public static void msgPrintln(String resource, Object[] params, TraceLevel traceLevel) 
    {
	if (traceLevelList.contains(traceLevel)) 
	{
	    // load the message resource
	    String message = ResourceManager.getMessage(resource);
	    String output = MessageFormat.format(message, params);
	    
	    TraceMessage tm = new TraceMessage(traceLevel, output, true);

	    // post message to the queue
	    enQueue(tm);
	}
    }

    public static void msgPrintln(String resource, Object[] params) {
	msgPrintln(resource, params, TraceLevel.DEFAULT);
    }
    
    // For old plugin Trace. set level to BASIC
    public static void msgPrintln(String resource) {
	msgPrintln(resource, null, TraceLevel.BASIC);
    }

    public static void addTraceListener(TraceListener tl) {
	synchronized(listenersList) {
	    if (!listenersList.contains(tl)) {
	        listenersList.add(tl);
	    }
	}
    }

    public static void removeTraceListener(TraceListener tl) {
	synchronized(listenersList) {
	    int index = listenersList.indexOf(tl);
	    if (index != -1) {
	        listenersList.remove(index);
	    }
	}
    }

    public static void resetTraceLevel() {
	traceLevelList.clear();

	// DEFAULT TraceLevel enabled by default
	traceLevelList.add(TraceLevel.DEFAULT);
    }

    static void firePrintlnEvent(TraceMessage tm) {
	TraceListener tl = null;
	// calls println of each registered listeners
	synchronized(listenersList) {
	    for (int i = 0; i < listenersList.size(); i++) {
	        tl = (TraceListener)listenersList.get(i);
	        tl.print(tm.getMessage());
	    }
        }
    }

    public static void ignoredException(Exception e) {
	Trace.ignored(e);
    }

    public static void ignored(Throwable t) {
	// print exception only if any Trace Level is enabled (besides DEFAULT)
	if (traceLevelList.size() > 1) {
	    t.printStackTrace();	    
	}
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void printException(Throwable e)
    {
	printException(null, e);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     */
    public static void printException(Component parentComponent, Throwable e)
    {
	printException(parentComponent, e, 
		       null,
		       null);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, String desc, String caption)
    {
	printException(parentComponent, e, desc, caption, true);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     * @param show Show exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, 
				      String desc, String caption, boolean show)
    {
	// Exception should be shown all the time
        println("Ignored exception: "+e); 

	// Show exception only if automation is disabled
	if (show && isAutomationEnabled() == false)
	{
	    if (desc == null)
		desc = ResourceManager.getMessage("dialogfactory.general_error");

	    // Show Exception dialog
	    UIFactory.showExceptionDialog(parentComponent, e, desc, caption);
	}
    }

    /** 
     * Output message to System.out if network tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void netPrintln(String msg)
    {
	println(msg, TraceLevel.NETWORK);
    }


    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgNetPrintln(String resource)
    {
	msgPrintln(resource, null, TraceLevel.NETWORK);
    }


    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgNetPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceLevel.NETWORK);
    }

    /** 
     * Output exception to System.out if net tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void netPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.net_error"), null, false);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void netPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, false);
    }


    /** 
     * Output message to System.out if security tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void securityPrintln(String msg)
    {
    	 println(msg, TraceLevel.SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     */
    public static void msgSecurityPrintln(String resource)
    {
    	 msgPrintln(resource, null, TraceLevel.SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     * @param params for the resource string
     */
    public static void msgSecurityPrintln(String resource, Object[] params)
    {
    	 msgPrintln(resource, params, TraceLevel.SECURITY);
    }

    /** 
     * Output exception to System.out if security tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void securityPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.security_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void securityPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if extension tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void extPrintln(String msg)
    {
	println(msg, TraceLevel.EXTENSIONS);
    }


    /** 
     * Output resource message to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgExtPrintln(String resource)
    {
	msgPrintln(resource, null, TraceLevel.EXTENSIONS);
    }


    /** 
     * Output resource to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgExtPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceLevel.EXTENSIONS);
    }

    /** 
     * Output exception to System.out if extension tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void extPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.ext_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void extPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if LiveConnect tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void liveConnectPrintln(String msg)
    {
	println(msg, TraceLevel.LIVECONNECT);
    }


    /** 
     * Output resource message to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgLiveConnectPrintln(String resource)
    {
	msgPrintln(resource, null, TraceLevel.LIVECONNECT);
    }


    /** 
     * Output resource to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgLiveConnectPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceLevel.LIVECONNECT);
    }

    /** 
     * Output exception to System.out if LiveConnect tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void liveConnectPrintException(Throwable e)
    {
	// Notice that we should never popup dialog in LiveConnect because
	// it may be called from the main thread, and we may end up deadlock
	// ourselves.

	printException(null, e, null, null, false);
    }

    public static File createTempFile(final String prefix, final String suffix, 
				      File directory)
    {
	try {
	    //Get the list of files that match this name
	    final File[] files = directory.listFiles(new FileFilter() {
		    public boolean accept(File pathname) {
			String filename = pathname.getName();
			return filename.startsWith(prefix) && 
			    filename.endsWith(suffix);
		    }
		});
	    
	    int iMaxNumOfFiles = Config.getIntProperty(Config.MAX_NUM_FILES_KEY);
	    
	    //If we have the maximum # of files, we have find the oldest and delete them
	    if ((iMaxNumOfFiles > 0) && (files.length >= iMaxNumOfFiles)) {
		final int numOfFiles = files.length;
		long[] modifiedTimeArray = new long[numOfFiles];
		for (int i=0; i < numOfFiles; i++) {
		    modifiedTimeArray[i]=files[i].lastModified();
		}  	   
		
		Arrays.sort(modifiedTimeArray);
		
		//Now delete the files that are older than the oldest allowed
		for (int i=0; i < modifiedTimeArray.length-iMaxNumOfFiles+1; i++)  {
		    files[i].delete();	    
		}
	    }		    
	    
	    return File.createTempFile(prefix, suffix, directory);
	} catch (Exception ioe) {
	    Trace.ignored(ioe);
	}
	return null;
    }
}
