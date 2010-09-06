/*
 * @(#)CommandSetNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class CommandSetNode extends AbstractNamedNode {

    void constrainComponent(Context ctx, Node node) {
        if (node instanceof CommandNode) {
            node.constrain(ctx);
        } else {
            error("Expected 'Command' item, got: " + node);
        }
    }

    void document(PrintWriter writer) {
        writer.println("<h4><a name=\"" + context.whereC + "\">" + name + 
                       " Command Set</a> (" + 
                       nameNode.value() + ")</h4>");
        writer.println(comment());
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).document(writer);
        }
    }

    void documentIndex(PrintWriter writer) {
        writer.print("<li><a href=\"#" + context.whereC + "\">");
        writer.println(name() + "</a> Command Set (" + 
                       nameNode.value() + ")");
        writer.println("<ul>");
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).documentIndex(writer);
        }
        writer.println("</ul>");
    }

    void genJavaClassSpecifics(PrintWriter writer, int depth) {
        indent(writer, depth);
        writer.println("static final int COMMAND_SET = " + nameNode.value() + ";");
        indent(writer, depth);
        writer.println("private " + name() + "() {}  // hide constructor");
    }

    void genJava(PrintWriter writer, int depth) {
        genJavaClass(writer, depth);
    }

}
