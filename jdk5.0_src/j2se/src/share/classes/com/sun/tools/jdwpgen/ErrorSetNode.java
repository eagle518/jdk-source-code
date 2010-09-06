/*
 * @(#)ErrorSetNode.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ErrorSetNode extends AbstractSimpleNode {

    void constrainComponent(Context ctx, Node node) {
        if (node instanceof ErrorNode) {
            node.constrain(ctx);
        } else {
            error("Expected 'Error' item, got: " + node);
        }
    }

    void document(PrintWriter writer) {
        
        writer.println("<dt>" + "Error Data");
        if (components.size() == 0) {
            writer.println("<dd>(None)");
        } else {
            writer.println("<dd><table border=1 cellpadding=3 cellspacing=0 width=\"90%\" summary=\"\">");
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).document(writer);
        }
        writer.println("</table>");
        }
    }

    void genJavaComment(PrintWriter writer, int depth) {}

    void genJava(PrintWriter writer, int depth) {}

    void genCInclude(PrintWriter writer) {}

    void genJavaDebugWrite(PrintWriter writer, int depth,
                           String writeLabel) {}

    void genJavaDebugWrite(PrintWriter writer, int depth,
                           String writeLabel, String displayValue) {}

    public void genJavaRead(PrintWriter writer, int depth,
                            String readLabel) {}
 
    void genJavaDebugRead(PrintWriter writer, int depth,
                          String readLabel, String displayValue) {}

    void genJavaPreDef(PrintWriter writer, int depth) {}

}
