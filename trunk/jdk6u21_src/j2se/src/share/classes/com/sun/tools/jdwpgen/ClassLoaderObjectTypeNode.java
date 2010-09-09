/*
 * @(#)ClassLoaderObjectTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ClassLoaderObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "classLoaderID";
    }

    String javaType() {
        return "ClassLoaderReferenceImpl";
    }

    String javaRead() {
        return "ps.readClassLoaderReference()";
    }
}
