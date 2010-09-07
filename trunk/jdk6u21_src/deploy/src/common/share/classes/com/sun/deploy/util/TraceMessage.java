/*
 * @(#)TraceMessage.java	1.8 09/01/10
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.util;

final class TraceMessage {

    private TraceLevel level;
    private String     msg;
    private boolean    newline;
    private static long ms = 0;

    TraceMessage(TraceLevel level, String msg) {
	this(level, msg, false);
    }

    TraceMessage(TraceLevel level, String msg, boolean newline) {
	this.level = level;
        if (msg.length() > 10240) {
            // adding huge strings to Console can cause hang in JTextArea
            msg = (msg.substring(0, 10239) + "\nTrace message truncated for length over 10K\n");
        }
	this.msg = msg;
	this.newline = newline;
    }

    private long getMs() {
       long time = System.currentTimeMillis();
       if (ms == 0) { 
           ms = time;
       }
       return (time - ms);
    }

    public String toString() {
	if (PerfLogger.perfLogEnabled()) {
	    return new StringBuffer().append("ts: ").append(getMs()).append(" ").append(level).append(": ").append(msg).toString();
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
