/*
 * @(#)InterfaceTypeNode.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
