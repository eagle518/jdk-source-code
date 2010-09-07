/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import sun.jvm.hotspot.oops.Symbol;
import sun.jvm.hotspot.oops.Oop;
import sun.jvm.hotspot.oops.InstanceKlass;

import java.util.List;

/**
 * There is no SA class that corresponds to this.  Therefore,
 * all the methods in this class which involve the SA mirror class
 * have to be implemented in the subclasses.
 */
abstract public class TypeComponentImpl extends MirrorImpl
    implements TypeComponent {

    protected final ReferenceTypeImpl declaringType;
    protected String signature;

    TypeComponentImpl(VirtualMachine vm, ReferenceTypeImpl declaringType) {
        super(vm);
        this.declaringType = declaringType;
    }

    public ReferenceType declaringType() {
        return declaringType;
    }

    public String signature() {
        return signature;
    }

    abstract public String name();
    abstract public int modifiers();
    abstract public boolean isPackagePrivate();
    abstract public boolean isPrivate();
    abstract public boolean isProtected();
    abstract public boolean isPublic();
    abstract public boolean isStatic();
    abstract public boolean isFinal();
    abstract public int hashCode();
}
