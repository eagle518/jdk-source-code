/*
 * @(#)JSExceptions.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.liveconnect;

import netscape.javascript.*;

/** Helps provide uniform JSException detail messages across browsers. */ 

public class JSExceptions {
    private JSExceptions() {
    }

    public static JSException noSuchMethod(String methodName) {
        return new JSException("No such method \"" + methodName + "\" on JavaScript object");
    }

    public static JSException noSuchProperty(String propertyName) {
        return new JSException("No such property \"" + propertyName + "\" on JavaScript object");
    }

    public static JSException noSuchSlot(int index) {
        return new JSException("No such slot " + index + " on JavaScript object");
    }

    public static JSException canNotRemoveMember(String memberName) {
        return new JSException("Member \"" + memberName + "\" does not exist, or exists and can not be removed");
    }
}
