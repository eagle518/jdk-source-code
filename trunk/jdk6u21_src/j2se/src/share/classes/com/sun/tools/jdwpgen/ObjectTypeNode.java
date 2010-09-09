/*
 * @(#)ObjectTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ObjectTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "objectID";
    }

    String javaType() {
        return "ObjectReferenceImpl";
    }

    String debugValue(String label) {
        return "(" + label + "==null?\"NULL\":\"ref=\"+" + label + ".ref())";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel, debugValue(writeLabel));
        indent(writer, depth);
        writer.println("ps.writeObjectRef(" + writeLabel + ".ref());");
    }

    String javaRead() {
        return "ps.readObjectReference()";
    }
}
