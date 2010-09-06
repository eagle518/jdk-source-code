/*
 * @(#)VMAction.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.tools.jdi;

import com.sun.jdi.*;
import java.util.EventObject;

/*
 * The name "action" is used to avoid confusion
 * with JDI events.
 */
class VMAction extends EventObject {
    // Event ids
    static final int VM_SUSPENDED = 1;
    static final int VM_NOT_SUSPENDED = 2;

    int id;

    VMAction(VirtualMachine vm, int id) {
        super(vm);
        this.id = id;
    }
    VirtualMachine vm() {
        return (VirtualMachine)getSource();
    }
    int id() {
        return id;
    }
}


