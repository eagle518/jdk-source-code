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

public abstract class TypeImpl extends MirrorImpl implements Type
{
    private String typeName;

    TypeImpl(VirtualMachine aVm) {
        super(aVm);
    }

    public abstract String signature();

    public String name() {
        if (typeName == null) {
            JNITypeParser parser = new JNITypeParser(signature());
            typeName = parser.typeName();
        }
        return typeName;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof Type)) {
            Type other = (Type)obj;
            return signature().equals(other.signature()) &&
                   super.equals(obj);
        } else {
            return false;
        }
    }

    public int hashCode() {
        return signature().hashCode();
    }
}
