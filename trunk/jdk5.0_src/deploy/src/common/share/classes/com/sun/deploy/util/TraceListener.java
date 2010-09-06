/*
 * @(#)TraceListener.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/**
 * TraceListener is an interface that acts as a basic trace listener.
 */
public interface TraceListener {
    /** 
     * Output message to listener.
     *
     * @param msg Message to be printed
     */
    public void print(String msg);
}
