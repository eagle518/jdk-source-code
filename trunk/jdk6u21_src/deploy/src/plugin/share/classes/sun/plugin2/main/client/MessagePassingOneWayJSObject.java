/*
 * @(#)MessagePassingOneWayJSObject.java	1.2 10/05/21
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.security.*;

import netscape.javascript.*;

import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.*;
import sun.plugin2.applet.Plugin2Manager;

// FIXME: need to support swapping in of the NoopExecutionContext to
// disallow LiveConnect for a particular applet

/** This is a wrapper of the MessagePassingJSObject. All methods calls are
 *  being delegated to the MessagePassingJSObject. This is for the purposes of
 *  tracking java to javascript calls initiated from the custom progress code.
 *  If the return object is non-null, it will be wrapped inside this object
 *  before returning. */

public class MessagePassingOneWayJSObject extends JSObject {
    private MessagePassingJSObject mpjso;
    private Plugin2Manager manager;
    private static final boolean DEBUG;

    static {
        DEBUG = (AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return SystemUtil.getenv("JPI_PLUGIN2_DEBUG");
                }
            }) != null);
    }

    public MessagePassingOneWayJSObject(MessagePassingJSObject mpjso) {
        this.mpjso = (MessagePassingJSObject)mpjso;
        this.manager = mpjso.getManager();
    }

    public BrowserSideObject getBrowserSideObject() {
        return mpjso.getBrowserSideObject();
    }

    public int getAppletID() {
        return mpjso.getAppletID();
    }

    public Object call(String methodName, Object args[]) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        Object returnObj = mpjso.call(methodName, args);

        if (returnObj == null) {
            return null;
        }

        if (ArgumentHelper.isPrimitiveOrString(returnObj)) {
            return returnObj;
        } else {
            return (Object)(new MessagePassingOneWayJSObject((MessagePassingJSObject)returnObj));
        }
    }

    public Object eval(String s) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        Object returnObj = mpjso.eval(s);

        if (returnObj == null) {
            return null;
        }

        if (ArgumentHelper.isPrimitiveOrString(returnObj)) {
            return returnObj;
        } else {
            return (Object)(new MessagePassingOneWayJSObject((MessagePassingJSObject)returnObj));
        }
    }

    public Object getMember(String name) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        Object returnObj = mpjso.getMember(name);

        if (returnObj == null) {
            return null;
        }

        if (ArgumentHelper.isPrimitiveOrString(returnObj)) {
            return returnObj;
        } else {
            return (Object)(new MessagePassingOneWayJSObject((MessagePassingJSObject)returnObj));
        }
    }

    public void setMember(String name, Object value) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        mpjso.setMember(name, value);
    }

    public void removeMember(String name) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        mpjso.removeMember(name);
    }

    public Object getSlot(int index) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        Object returnObj = mpjso.getSlot(index);

        if (returnObj == null) {
            return null;
        }

        if (ArgumentHelper.isPrimitiveOrString(returnObj)) {
            return returnObj;
        } else {
            return (Object)(new MessagePassingOneWayJSObject((MessagePassingJSObject)returnObj));
        }
    }

    public void setSlot(int index, Object value) throws JSException {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        mpjso.setSlot(index, value);
    }

    public String toString() {
        if (manager != null) {
            manager.increaseJava2JSCounter();
        }

        return mpjso.toString();
    }

}
