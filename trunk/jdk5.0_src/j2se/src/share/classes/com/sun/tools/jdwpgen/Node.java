/*
 * @(#)Node.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;


import java.util.*;
import java.io.*;

abstract class Node {

    String kind;
    List components;
    int lineno;
    List commentList = new ArrayList();
    Node parent = null;
    Context context;

    static final int maxStructIndent = 5;
    static int structIndent = 0; // horrible hack

    abstract void document(PrintWriter writer);

    void set(String kind, List components, int lineno) {
        this.kind = kind;
        this.components = components;
        this.lineno = lineno;
    }

    void parentAndExtractComments() {
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            if (node instanceof CommentNode) {
                it.remove();
                commentList.add(((CommentNode)node).text());
            } else {
                node.parent = this;
                node.parentAndExtractComments();
            }
        }
    }

    void prune() {
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            node.prune();
        }
    }

    void constrain(Context ctx) {
        context = ctx;
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            constrainComponent(ctx, node);
        }
    }

    void constrainComponent(Context ctx, Node node) {
        node.constrain(ctx);
    }

    void indent(PrintWriter writer, int depth) {
        for (int i = depth; i > 0; --i) {
            writer.print("    ");
        }
    }

    void documentIndex(PrintWriter writer) {
    }

    void docRowStart(PrintWriter writer) {
        writer.println("<tr>");
        if (structIndent > 0) {
            writer.println("<td colspan=" + structIndent + ">");
        }
    }

    String comment() {
        StringBuffer comment = new StringBuffer();
        for (Iterator it = commentList.iterator(); it.hasNext();) {
            comment.append((String)it.next());
        }
        return comment.toString();
    }

    void genJavaComment(PrintWriter writer, int depth) {
        if (commentList.size() > 0) {
            indent(writer, depth);
            writer.println("/**");
            for (Iterator it = commentList.iterator(); it.hasNext();) {
                indent(writer, depth);
                writer.println(" * " + (String)it.next());
            }
            indent(writer, depth);
            writer.println(" */");
        }
    }

    String javaType() {
        return "-- WRONG ---";
    }

    void genJava(PrintWriter writer, int depth) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            node.genJava(writer, depth);
        }
    }

    void genCInclude(PrintWriter writer) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            node.genCInclude(writer);
        }
    }

    String debugValue(String label) {
        return label;
    }

    void genJavaDebugWrite(PrintWriter writer, int depth, 
                           String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel, debugValue(writeLabel));
    }

    void genJavaDebugWrite(PrintWriter writer, int depth, 
                           String writeLabel, String displayValue) {
        if (!Main.genDebug) {
            return;
        }
        indent(writer, depth);
        writer.println(
          "if ((ps.vm.traceFlags & VirtualMachineImpl.TRACE_SENDS) != 0) {");
        indent(writer, depth+1);
        writer.print("ps.vm.printTrace(\"Sending: ");
        indent(writer, depth);  // this is inside the quotes
        writer.print(writeLabel + "(" + javaType() + "): \" + ");
        writer.println(displayValue + ");");
        indent(writer, depth);
        writer.println("}");
    }

    public void genJavaRead(PrintWriter writer, int depth, 
                            String readLabel) {
        error("Internal - Should not call Node.genJavaRead()");
    }
 
    void genJavaDebugRead(PrintWriter writer, int depth, 
                          String readLabel, String displayValue) {
        if (!Main.genDebug) {
            return;
        }
        indent(writer, depth);
        writer.println(
          "if (vm.traceReceives) {");
        indent(writer, depth+1);
        writer.print("vm.printReceiveTrace(" + depth + ", \"");
        writer.print(readLabel + "(" + javaType() + "): \" + ");
        writer.println(displayValue + ");");
        indent(writer, depth);
        writer.println("}");
    }

    void genJavaPreDef(PrintWriter writer, int depth) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            Node node = (Node)it.next();
            node.genJavaPreDef(writer, depth);
        }
    }

    void error(String errmsg) {
        System.err.println();
        System.err.println(Main.specSource + ":" + lineno + ": " + 
                           kind + " - " + errmsg);
        System.err.println();
        System.exit(1);
    }
}
