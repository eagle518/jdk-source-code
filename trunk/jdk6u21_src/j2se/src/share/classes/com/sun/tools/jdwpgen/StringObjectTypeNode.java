/*
 * @(#)StringObjectTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class StringObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "stringID";
    }

    String javaType() {
        return "StringReferenceImpl";
    }

    String javaRead() {
        return "ps.readStringReference()";
    }
}
