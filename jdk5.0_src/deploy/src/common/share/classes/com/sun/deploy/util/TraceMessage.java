/*
 * @(#)TraceMessage.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.util;

final class TraceMessage {

    private TraceLevel level;
    private String     msg;
    private boolean    newline;

    TraceMessage(TraceLevel level, String msg) {
	this(level, msg, false);
    }

    TraceMessage(TraceLevel level, String msg, boolean newline) {
	this.level = level;
	this.msg = msg;
	this.newline = newline;
    }

    public String toString() {
	if (PerfLogger.perfLogEnabled()) {
	    return new StringBuffer().append("ts: ").append(System.currentTimeMillis()).append(" ").append(level).append(": ").append(msg).toString();
	} else {
	    return new StringBuffer().append(level).append(": ").append(msg).toString();
	}
    }

    String getMessage() {
	StringBuffer message = new StringBuffer();

	if (PerfLogger.perfLogEnabled()) {
	    message.append("ts: ").append(System.currentTimeMillis()).append(" ");
	}
	// do not append TraceLevel information for DEFAULT level, since
	// it is output from application/applet.  We should just print out
	// whatever the user passed in.
	if (level.equals(TraceLevel.DEFAULT)) {
	    message.append(msg);
	} else {
	    message.append(level).append(": ").append(msg);
	}

	if (newline) {
	    message.append("\n");
	} 

	return message.toString();
    }
}
