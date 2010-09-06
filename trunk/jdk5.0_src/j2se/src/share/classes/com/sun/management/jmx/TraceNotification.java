/*
 * @(#)TraceNotification.java	1.6 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.management.jmx;


import java.lang.*;

import javax.management.*;


/**
 * This class defines an object used by the class Trace to send out
 * all internal runtime information.
 *
 * @deprecated Use {@link java.util.logging} APIs instead.
 *
 * @since 1.5
 */
@Deprecated
public class TraceNotification extends Notification {

    /**
     * The level of information.
     */
    public int level;
    
    /**
     * The type of information.
     */
    public int type;
    
    /**
     * The name of the class from which the information comes
     */
    public String className;
    
    /**
     * The name of the method from which the information comes.
     */
    public String methodName;
    
    /**
     * The information sent out by the class Trace.
     * <P>It can be null if an exception is provided as information.
     */
    public String info;
    
    /**
     * The exception sent out by the class Trace.
     * <P>It can be null if a string is provided as information.
     */
    public Throwable exception;
    
    /**
     * Global sequence number representing the place of this notification in all
     * notification sequence.
     */
    public long globalSequenceNumber;
    
    /**
     * Sequence number representing the place of this notification in the sequence of
     * all same type notifications
     */
    public long sequenceNumber;
    
    
    /**
     * Construct a TraceNotification object.
     *
     * @param source the emitter of the notification.
     * @param sequenceNumber the sequence number representing the place of this notification in the sequence of all same type notifications.
     * @param globalSequenceNumber the global sequence number representing the place of this notification in all notifications sent out by the class Trace.
     * @param level the level of information.
     * @param type the type of the information.
     * @param className the name of the class from which the information is from.
     * @param methodName the name of the method from which the information is from.
     * @param info an string as the trace information.
     * @param exception an exception as the trace information.
     */
    public TraceNotification(Object source,
			     long sequenceNumber,
			     long globalSequenceNumber,
			     int level,
			     int type,
			     String className,
			     String methodName,
			     String info,
			     Throwable exception) {
	super(null, source, sequenceNumber);
	
	this.sequenceNumber = sequenceNumber;
	this.globalSequenceNumber = globalSequenceNumber;
	this.level = level;
	this.type = type;
	this.className = (className != null) ? new String(className) : "";
	this.methodName = (methodName != null) ? new String(methodName) : "";
	this.info = (info != null) ? new String(info) : null;
	this.exception = exception;
    }
}
