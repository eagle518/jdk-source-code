/*
 * @(#)TraceLevel.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.util;

public class TraceLevel {

    public static final TraceLevel DEFAULT     = new TraceLevel("default");
    public static final TraceLevel BASIC       = new TraceLevel("basic");
    public static final TraceLevel NETWORK     = new TraceLevel("network");
    public static final TraceLevel SECURITY    = new TraceLevel("security");
    public static final TraceLevel CACHE       = new TraceLevel("cache");
    public static final TraceLevel EXTENSIONS  = new TraceLevel("extensions");
    public static final TraceLevel LIVECONNECT = new TraceLevel("liveconnect");
    public static final TraceLevel TEMP        = new TraceLevel("temp");

    private final String level;

    private TraceLevel(String level) {
	this.level = level;
    }

    public String toString() {
	return level;
    }

}
