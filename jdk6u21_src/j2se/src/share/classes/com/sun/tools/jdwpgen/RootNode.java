/*
 * @(#)RootNode.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class RootNode extends AbstractNamedNode {

    void constrainComponent(Context ctx, Node node) {
        if (node instanceof CommandSetNode || 
                    node instanceof ConstantSetNode) {
            node.constrain(ctx);
        } else {
            error("Expected 'CommandSet' item, got: " + node);
        }
    }

    void document(PrintWriter writer) {
        writer.println("<html><head><title>" + comment() + "</title></head>");
        writer.println("<body bgcolor=\"white\">");
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).documentIndex(writer);
        }
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).document(writer);
        }
        writer.println("</body></html>");
    }

    void genJava(PrintWriter writer, int depth) {
        writer.println("package com.sun.tools.jdi;");
        writer.println();
        writer.println("import com.sun.jdi.*;");
        writer.println("import java.util.*;");
        writer.println();

        genJavaClass(writer, depth);
    }
}
