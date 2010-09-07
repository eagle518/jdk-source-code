/*
 * @(#)TraceListener.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
