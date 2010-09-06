/*
 * @(#)AbstractTypeNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;


import java.util.*;
import java.io.*;

abstract class AbstractTypeNode extends AbstractNamedNode 
                                implements TypeNode {

    abstract String docType();

    public abstract void genJavaWrite(PrintWriter writer, int depth, 
                                      String writeLabel);

    abstract String javaRead();

    void document(PrintWriter writer) {
        docRowStart(writer);
        writer.println("<td colspan=" + 
                       (maxStructIndent - structIndent) + ">");
        writer.println(docType() + "<td><i>" + name() + 
                       "</i><td>" + comment() + "&nbsp;");
    }

    String javaType() {
        return docType(); // default
    }

    public void genJavaRead(PrintWriter writer, int depth, 
                            String readLabel) {
        indent(writer, depth);
        writer.print(readLabel);
        writer.print(" = ");
        writer.print(javaRead());
        writer.println(";");
        genJavaDebugRead(writer, depth, readLabel, debugValue(readLabel));
    }

    public void genJavaDeclaration(PrintWriter writer, int depth) {
        writer.println();
        genJavaComment(writer, depth);
        indent(writer, depth);
        writer.print("final ");
        writer.print(javaType());
        writer.print(" " + name);
        writer.println(";");
    }

    public String javaParam() {
        return javaType() + " " + name;
    }
}
