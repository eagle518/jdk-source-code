/*
 * @(#)ReplyNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ReplyNode extends AbstractTypeListNode {

    String cmdName;

    void set(String kind, List components, int lineno) {
        super.set(kind, components, lineno);
        components.add(0, new NameNode(kind));
    }

    void constrain(Context ctx) {
        super.constrain(ctx.replyReadingSubcontext());
        CommandNode cmd = (CommandNode)parent;
        cmdName = cmd.name;
    }

    void genJava(PrintWriter writer, int depth) {
        genJavaPreDef(writer, depth);
        super.genJava(writer, depth);
        writer.println();
        genJavaReadingClassBody(writer, depth, cmdName);
    }

    void genJavaReads(PrintWriter writer, int depth) {
        if (Main.genDebug) {
            indent(writer, depth);
            writer.println(
                "if (vm.traceReceives) {");
            indent(writer, depth+1);
            writer.print(
                "vm.printTrace(\"Receiving Command(id=\" + ps.pkt.id + \") "); 
            writer.print(parent.context.whereJava);
            writer.print("\"");
            writer.print(
                "+(ps.pkt.flags!=0?\", FLAGS=\" + ps.pkt.flags:\"\")");
            writer.print(
                "+(ps.pkt.errorCode!=0?\", ERROR CODE=\" + ps.pkt.errorCode:\"\")");
            writer.println(");");
            indent(writer, depth);
            writer.println("}");
        }
        super.genJavaReads(writer, depth);
    }
}
