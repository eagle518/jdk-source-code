/*
 * @(#)ReferenceTypeNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ReferenceTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "referenceTypeID";
    }

    String javaType() {
        return "ReferenceTypeImpl";
    }

    String debugValue(String label) {
        return "(" + label + "==null?\"NULL\":\"ref=\"+" + label + ".ref())";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel, 
                          debugValue(writeLabel));
        indent(writer, depth);
        writer.println("ps.writeClassRef(" + writeLabel + ".ref());");
    }

    String javaRead() {
        error("--- should not gen ---");
        return null;
    }
}
