/*
 * @(#)OutNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class OutNode extends AbstractTypeListNode {

    String cmdName;

    void set(String kind, List components, int lineno) {
        super.set(kind, components, lineno);
        components.add(0, new NameNode("Out"));
    }

    void constrain(Context ctx) {
        super.constrain(ctx.commandWritingSubcontext());
        CommandNode cmd = (CommandNode)parent;
        cmdName = cmd.name;
    }

    void genProcessMethod(PrintWriter writer, int depth) {
        writer.println();
        indent(writer, depth);
        writer.print(
            "static " + cmdName + " process(VirtualMachineImpl vm");
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            writer.println(", ");
            indent(writer, depth+5);
            writer.print(tn.javaParam());
        }
        writer.println(")");
        indent(writer, depth+6);
        writer.println("throws JDWPException {");
        indent(writer, depth+1);
        writer.print("PacketStream ps = enqueueCommand(vm");
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            writer.print(", ");
            writer.print(tn.name());
        }
        writer.println(");");
        indent(writer, depth+1);
        writer.println("return waitForReply(vm, ps);");
        indent(writer, depth);
        writer.println("}");
    }

    void genEnqueueMethod(PrintWriter writer, int depth) {
        writer.println();
        indent(writer, depth);
        writer.print(
            "static PacketStream enqueueCommand(VirtualMachineImpl vm");
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            writer.println(", ");
            indent(writer, depth+5);
            writer.print(tn.javaParam());
        }
        writer.println(") {");
        indent(writer, depth+1);
        writer.println(
            "PacketStream ps = new PacketStream(vm, COMMAND_SET, COMMAND);");
        if (Main.genDebug) {
            indent(writer, depth+1);
            writer.println(
                "if ((vm.traceFlags & vm.TRACE_SENDS) != 0) {");
            indent(writer, depth+2);
            writer.print(
                "vm.printTrace(\"Sending Command(id=\" + ps.pkt.id + \") "); 
            writer.print(parent.context.whereJava);
            writer.println(
                "\"+(ps.pkt.flags!=0?\", FLAGS=\" + ps.pkt.flags:\"\"));");
            indent(writer, depth+1);
            writer.println("}");
        }
        genJavaWrites(writer, depth+1);
        indent(writer, depth+1);
        writer.println("ps.send();");
        indent(writer, depth+1);
        writer.println("return ps;");
        indent(writer, depth);
        writer.println("}");
    }

    void genWaitMethod(PrintWriter writer, int depth) {
        writer.println();
        indent(writer, depth);
        writer.println(
            "static " + cmdName + " waitForReply(VirtualMachineImpl vm, " +
                                  "PacketStream ps)");
        indent(writer, depth+6);
        writer.println("throws JDWPException {");
        indent(writer, depth+1);
        writer.println("ps.waitForReply();");
        indent(writer, depth+1);
        writer.println("return new " + cmdName + "(vm, ps);");
        indent(writer, depth);
        writer.println("}");
    }

    void genJava(PrintWriter writer, int depth) {
        genJavaPreDef(writer, depth);
        super.genJava(writer, depth);
        genProcessMethod(writer, depth);
        genEnqueueMethod(writer, depth);
        genWaitMethod(writer, depth);
    }
}

