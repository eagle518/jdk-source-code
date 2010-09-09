/*
 * @(#)ValueTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ValueTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "value";
    }

    String javaType() {
        return "ValueImpl";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel);
        indent(writer, depth);
        writer.println("ps.writeValue(" + writeLabel + ");");
    }

    String javaRead() {
        return "ps.readValue()";
    }
}
