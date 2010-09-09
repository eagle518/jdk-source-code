/*
 * @(#)ArrayTypeNode.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ArrayTypeNode extends ReferenceTypeNode {

    String docType() {
        return "arrayTypeID";
    }

    String javaType() {
        return "ArrayTypeImpl";
    }

    String javaRead() {
        return "--- should not get generated ---";
    }
}
