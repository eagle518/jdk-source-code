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

abstract class MirrorImpl extends Object implements Mirror {
    protected VirtualMachineImpl vm;

    MirrorImpl(VirtualMachine aVm) {
        super();

        // Yes, its a bit of a hack. But by doing it this
        // way, this is the only place we have to change
        // typing to substitute a new impl.
        vm = (VirtualMachineImpl)aVm;
    }

    public VirtualMachine virtualMachine() {
        return vm;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof Mirror)) {
            Mirror other = (Mirror)obj;
            return vm.equals(other.virtualMachine());
        } else {
            return false;
        }
    }

    public int hashCode() {
        return vm.hashCode();
    }
}
