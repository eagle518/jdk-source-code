/*
 * @(#)TraceLevel.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
