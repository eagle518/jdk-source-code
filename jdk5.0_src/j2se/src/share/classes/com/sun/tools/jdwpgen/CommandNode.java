/*
 * @(#)CommandNode.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class CommandNode extends AbstractCommandNode {

    void constrain(Context ctx) {
        if (components.size() == 3) {
            Node out = (Node)components.get(0);
            Node reply = (Node)components.get(1);
            Node error = (Node)components.get(2);
            if (!(out instanceof OutNode)) {
                error("Expected 'Out' item, got: " + out);
            }
            if (!(reply instanceof ReplyNode)) {
                error("Expected 'Reply' item, got: " + reply);
            }
            if (!(error instanceof ErrorSetNode)) {
                error("Expected 'ErrorSet' item, got: " + error);
            }
        } else if (components.size() == 1) {
            Node evt = (Node)components.get(0);
            if (!(evt instanceof EventNode)) {
                error("Expected 'Event' item, got: " + evt);
            }
        } else {
            error("Command must have Out and Reply items or ErrorSet item");
        }
        super.constrain(ctx);
    }

    void genJavaClassSpecifics(PrintWriter writer, int depth) {
        indent(writer, depth);
        writer.println("static final int COMMAND = " +
                       nameNode.value() + ";");
    }

    void genJava(PrintWriter writer, int depth) {
        genJavaClass(writer, depth);
    }
}
