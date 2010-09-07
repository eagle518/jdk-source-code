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
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.Klass;
import sun.jvm.hotspot.oops.OopUtilities;

public class ClassObjectReferenceImpl extends ObjectReferenceImpl
                                      implements ClassObjectReference {
    private ReferenceType reflectedType;

    ClassObjectReferenceImpl(VirtualMachine vm, Instance oRef) {
        super(vm, oRef);
    }

    public ReferenceType reflectedType() {
        if (reflectedType == null) {
            Klass k = OopUtilities.classOopToKlass(ref());
            reflectedType = vm.referenceType(k);
        }
        return reflectedType;
    }

    public String toString() {
        return "instance of " + referenceType().name() +
               "(reflected class=" + reflectedType().name() + ", " + "id=" +
               uniqueID() + ")";
    }
}
