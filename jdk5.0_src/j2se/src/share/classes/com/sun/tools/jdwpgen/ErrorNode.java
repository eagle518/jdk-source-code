/*
 * @(#)ErrorNode.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ErrorNode extends AbstractCommandNode {

    protected static final String NAME_OF_ERROR_TABLE = "Error";
    
    ErrorNode() {
        this(new ArrayList());
    }

    ErrorNode(List components) {
        this.kind = "Error";
        this.components = components;
        this.lineno = 0;
    }

    void constrain(Context ctx) {
        if (components.size() != 0) {
            error("Errors have no internal structure");
        }
        super.constrain(ctx);
    }

    void document(PrintWriter writer) {
        
        String com = comment();
        if (com == null || com.length() == 0) {
            com = ConstantSetNode.getConstant("Error_" + name);
        }
        writer.println("<tr><td>" + "<a href=\"#" + NAME_OF_ERROR_TABLE + "_" + name + "\">"
                       + name + "</a></td>" +
                       "<td>" + com + "&nbsp;</td></tr>");
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
