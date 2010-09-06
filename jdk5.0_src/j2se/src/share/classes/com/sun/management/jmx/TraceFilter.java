/*
 * @(#)TraceFilter.java	1.6 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.management.jmx;


import javax.management.*;

/**
 * This class does nothing. To get the traces, use the 
 * {@link java.util.logging} APIs.
 *
 * @deprecated Use {@link java.util.logging} APIs instead.
 *
 * @since 1.5
 */
@Deprecated
public class TraceFilter implements NotificationFilter {

    // -----------------
    // protected variables
    // -----------------
    protected int levels;
    protected int types;

// ------------
// constructors
// ------------
    /**
     * This class does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public TraceFilter(int levels, int types) throws IllegalArgumentException {
	this.levels = levels;
	this.types = types;
    }


// --------------
// public methods
// --------------
    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     * @return false
     */
    public boolean isNotificationEnabled(Notification notification) {
	return false;
    }

    /**
     * get the levels selected
     *
     * @return the levels selected for filtering.
     */
    public int getLevels() {
	return levels;
    }
    
    /**
     * get types selected
     *
     * @return the types selected for filtering.
     */
    public int getTypes() {
	return types;
    }
    
}
