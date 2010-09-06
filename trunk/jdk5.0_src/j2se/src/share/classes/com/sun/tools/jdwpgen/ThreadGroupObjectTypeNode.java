/*
 * @(#)ThreadGroupObjectTypeNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ThreadGroupObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "threadGroupID";
    }

    String javaType() {
        return "ThreadGroupReferenceImpl";
    }

    String javaRead() {
        return "ps.readThreadGroupReference()";
    }
}
