/*
 * @(#)AbstractTypeListNode.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

abstract class AbstractTypeListNode extends AbstractNamedNode {

    void constrainComponent(Context ctx, Node node) {
        if (node instanceof TypeNode) {
            node.constrain(ctx);
        } else {
            error("Expected type descriptor item, got: " + node);
        }
    }

    void document(PrintWriter writer) {
        writer.println("<dt>" + name() + " Data");
        if (components.size() == 0) {
            writer.println("<dd>(None)");
        } else {
            writer.println("<dd><table border=1 cellpadding=3 cellspacing=0 width=\"90%\" summary=\"\"><tr>");
            for (int i = maxStructIndent; i > 0; --i) {
                writer.print("<th width=\"4%\">");
            }
            writer.println("<th width=\"15%\"><th width=\"65%\">");
            writer.println("");
            for (Iterator it = components.iterator(); it.hasNext();) {
                ((Node)it.next()).document(writer);
            }
            writer.println("</table>");
        }
    }

    void genJavaClassBodyComponents(PrintWriter writer, int depth) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();

            tn.genJavaDeclaration(writer, depth);
        }
    }
  
    void genJavaReads(PrintWriter writer, int depth) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            tn.genJavaRead(writer, depth, tn.name());
        }
    }

    void genJavaReadingClassBody(PrintWriter writer, int depth, 
                                 String className) {
        genJavaClassBodyComponents(writer, depth);
        writer.println();
        indent(writer, depth);
        if (!context.inEvent()) {
            writer.print("private ");
        }
        writer.println(className + 
                       "(VirtualMachineImpl vm, PacketStream ps) {");
        genJavaReads(writer, depth+1);
        indent(writer, depth);
        writer.println("}");
    }

    String javaParams() {
        StringBuffer sb = new StringBuffer();
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            sb.append(tn.javaParam());
            if (it.hasNext()) {
                sb.append(", ");
            }
        }
        return sb.toString();
    }
  
    void genJavaWrites(PrintWriter writer, int depth) {
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            tn.genJavaWrite(writer, depth, tn.name());
        }
    }

    void genJavaWritingClassBody(PrintWriter writer, int depth, 
                                 String className) {
        genJavaClassBodyComponents(writer, depth);
        writer.println();
        indent(writer, depth);
        writer.println(className + "(" + javaParams() + ") {");
        for (Iterator it = components.iterator(); it.hasNext();) {
            TypeNode tn = (TypeNode)it.next();
            indent(writer, depth+1);
            writer.println("this." + tn.name() + " = " + tn.name() + ";");
        }
        indent(writer, depth);
        writer.println("}");
    }
}
