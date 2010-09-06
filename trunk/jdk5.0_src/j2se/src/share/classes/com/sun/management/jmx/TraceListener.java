/*
 * @(#)TraceListener.java	1.6 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management.jmx;


import java.lang.*;
import java.io.*;

import javax.management.*;


/**
 * An object of this class can be used to receive notifications sent out by the class Trace,
 * and all notifications received will be saved to a file specified, or write to user screen.
 * This listener will print or save information with two formats:
 * <UL><LI> simple format:
 * <P>(className methodName) Message.
 * This is a default format</LI>
 * <LI>completed format: all realative information is printed or saved like:
 * <P>Global sequence number: 7     Sequence number: 3
 * <P>Level: LEVEL_TRACE     Type: INFO_CONNECTOR_RMI
 * <P>Class  Name: MBeanServer
 * <P>Method Name: sendTraceInfo
 * <P>Information: This is a example to send a trace information.
 * <P>A user should call the method setFormated with the value "true" to select this format.
 *</LI></UL>
 *
 * <P>This class can be used as a default listener, a user can write his own listener to
 * treat trace information the way he wants. 
 *
 * @deprecated Use {@link java.util.logging} APIs instead.
 *
 * @since 1.5
 */
@Deprecated
public class TraceListener implements NotificationListener {

 
    // -------------
    // protected variables
    // -------------
    protected PrintStream out;
    protected boolean needTobeClosed = false;    
    protected boolean formated	= false;

// ------------
// constructors
// ------------
    /**
     * Construct a default trace listener.
     * All information will printed on screen (System.out) with the format:
     * <P> (className methodName) message.
     */
    public TraceListener() {
	out = System.out;
    }
    
    /**
     * Construct a trace listener with a specified output stream.
     *
     * @param ps a PrintStream object used to print trace information.
     * <P>For example, it can take value as System.out or System.err.
     *
     * @param ps An PrintStream object used to print out trace information.
     * @exception IllegalArgumentException thrown if the parameter ps is null.
     */
    public TraceListener(PrintStream ps) throws IllegalArgumentException {
	if (ps == null)
	    throw new IllegalArgumentException("An PrintStream object should be specified.");	
	out = ps;
    }
    
    /**
     * Construct a trace listener with a file specified to
     * save all information received.
     * <P>If specified file exists currently, all trace information will be appended to this file.
     *
     * @param fileName the file used to save information.
     * @exception IOException thrown if failed to open or write the file.
     */
    public TraceListener(String fileName) throws IOException {
	out = new PrintStream(new FileOutputStream(fileName, true));
	
	needTobeClosed = true;
    }
    
    // --------------
    // public methods
    // --------------
    /**
     * Choose a format to output trace information.
     *
     * @param f if true, the completed format will be selected.
     */
    public void setFormated (boolean f) {
	formated = f;
    }
    
    /**
     * Called by the class Trace to receive trace information.
     */
    public void handleNotification(Notification notif, Object handback) {
	if (notif instanceof TraceNotification) {
	    TraceNotification evt = (TraceNotification)notif;
	    
	    if (formated) {
		out.print("\nGlobal sequence number: " +evt.globalSequenceNumber+ "     Sequence number: " +evt.sequenceNumber+ "\n"
			  + "Level: " + Trace.getLevel(evt.level)+ "     Type: " +Trace.getType(evt.type)+ "\n"
			  + "Class  Name: " + new String(evt.className) + "\n"
			  + "Method Name: " + new String(evt.methodName) + "\n");
		
		if (evt.exception != null) {
		    evt.exception.printStackTrace(out);
		    out.println();
		}
		
		if (evt.info != null)
		    out.println("Information: " + evt.info);
	    } else {
		out.print("(" +evt.className + " " +evt.methodName+ ") ");
		
		if (evt.exception != null) {
		    evt.exception.printStackTrace(out);
		    out.println();
		}
		
		if (evt.info != null)
		    out.println(evt.info);
	    }
	}
    }
    
    /**
     * Specify a file to save information received
     *
     * @param fileName the file used to save information. It will
     * replace the old file or the PrintStream object setted before.
     * @exception IOException thrown if failed to open or to write the file.
     */
    public void setFile(String fileName) throws IOException {
	PrintStream newOut = new PrintStream(new FileOutputStream(fileName, true));
	
	if (needTobeClosed)
	    out.close();
	
	out = newOut;
	needTobeClosed = true;
    }
   
}
