/*
 * @(#)VoidTypeImpl.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class VoidTypeImpl extends TypeImpl implements VoidType {
    VoidTypeImpl(VirtualMachine vm) {
        super(vm);
    }

    public String signature() {
        return String.valueOf((char)JDWP.Tag.VOID);
    }

    public String toString() {
        return name();
    }
}
