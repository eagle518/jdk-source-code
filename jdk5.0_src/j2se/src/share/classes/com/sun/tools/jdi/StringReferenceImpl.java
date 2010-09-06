/*
 * @(#)StringReferenceImpl.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class StringReferenceImpl extends ObjectReferenceImpl
    implements StringReference
{
    private String value;

    StringReferenceImpl(VirtualMachine aVm,long aRef) {
        super(aVm,aRef);
    }

    public String value() {
        if(value == null) {
            // Does not need synchronization, since worst-case
            // static info is fetched twice
            try {
                value = JDWP.StringReference.Value.
                    process(vm, this).stringValue;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
        }
        return value;
    }

    public String toString() {
        return "\"" + value() + "\"";
    }

    byte typeValueKey() {
        return JDWP.Tag.STRING;
    }
}
