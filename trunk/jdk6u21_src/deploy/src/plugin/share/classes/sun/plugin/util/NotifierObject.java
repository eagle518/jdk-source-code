/*
 * @(#)NotifierObject.java	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.util;
 
/** A simple object which you can send a notification to which another
    thread can observe. */
 
public class NotifierObject {
    private volatile boolean notified = false;
 
    public NotifierObject() {
    }
 
    public void setNotified() {
        notified = true;
    }
 
    public boolean getNotified() {
        return notified;
    }
}
