/*
 * @(#)ReferenceIDTypeNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ReferenceIDTypeNode extends ReferenceTypeNode {

    String javaType() {
        return "long";
    }

    String debugValue(String label) {
        return "\"ref=\"+" + label;
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel);
        indent(writer, depth);
        writer.println("ps.writeClassRef(" + writeLabel + ");");
    }

    String javaRead() {
        return "ps.readClassRef()";
    }
}
