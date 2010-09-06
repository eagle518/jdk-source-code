/*
 * @(#)ObsoleteMethodImpl.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

import java.util.List;
import java.util.ArrayList;

/**
 * Represents methods which have changed when the class was redefined.
 */
public class ObsoleteMethodImpl extends NonConcreteMethodImpl {

    private Location location = null;

    ObsoleteMethodImpl(VirtualMachine vm, 
                          ReferenceTypeImpl declaringType) {
        super(vm, declaringType, 0, "<obsolete>", "", null, 0);
    }

    public boolean isObsolete() {
        return true;
    }

    public String returnTypeName() {
        return "<unknown>";
    }

    public Type returnType() throws ClassNotLoadedException {
        throw new ClassNotLoadedException("type unknown");
    }

    public List argumentTypeNames() {
        return new ArrayList();
    }

    public List argumentSignatures() {
        return new ArrayList();
    }

    Type argumentType(int index) throws ClassNotLoadedException {
        throw new ClassNotLoadedException("type unknown");
    }

    public List argumentTypes() throws ClassNotLoadedException {
        return new ArrayList();
    }

}
