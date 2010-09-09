/*
 * @(#)TaggedObjectTypeNode.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
