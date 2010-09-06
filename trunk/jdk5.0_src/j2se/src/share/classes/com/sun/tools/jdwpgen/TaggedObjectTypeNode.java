/*
 * @(#)TaggedObjectTypeNode.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class TaggedObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "tagged-objectID";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        error("Why write a tagged-object?");
    }

    String javaRead() {
        return "ps.readTaggedObjectReference()";
    }
}
