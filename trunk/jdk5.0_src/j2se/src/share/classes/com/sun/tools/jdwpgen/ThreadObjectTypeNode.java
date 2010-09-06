/*
 * @(#)ThreadObjectTypeNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ThreadObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "threadID";
    }

    String javaType() {
        return "ThreadReferenceImpl";
    }

    String javaRead() {
        return "ps.readThreadReference()";
    }
}
