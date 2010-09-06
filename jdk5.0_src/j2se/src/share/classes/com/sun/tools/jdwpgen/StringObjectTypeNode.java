/*
 * @(#)StringObjectTypeNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
