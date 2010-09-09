/*
 * @(#)FrameTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class FrameTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "frameID";
    }

    String javaType() {
        return "long";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel);
        indent(writer, depth);
        writer.println("ps.writeFrameRef(" + writeLabel + ");");
    }

    String javaRead() {
        return "ps.readFrameRef()";
    }
}
