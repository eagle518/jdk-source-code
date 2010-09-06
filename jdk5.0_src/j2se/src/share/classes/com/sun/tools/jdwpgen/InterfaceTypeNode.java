/*
 * @(#)InterfaceTypeNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class InterfaceTypeNode extends ReferenceTypeNode {

    String docType() {
        return "interfaceID";
    }

    String javaType() {
        return "InterfaceTypeImpl";
    }

    String javaRead() {
        return "vm.interfaceType(ps.readClassRef())";
    }
}
