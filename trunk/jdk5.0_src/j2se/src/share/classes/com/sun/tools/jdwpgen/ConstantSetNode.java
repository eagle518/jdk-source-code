/*
 * @(#)ConstantSetNode.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ConstantSetNode extends AbstractNamedNode {

    /**
     * The mapping between a constant and its value.
     */
    protected static Map constantMap;
    
    ConstantSetNode(){
        if (constantMap == null) {
            constantMap = new HashMap();
        }
    }
    
    void prune() {
        List addons = new ArrayList();

        for (Iterator it = components.iterator(); it.hasNext(); ) {
            Node node = (Node)it.next();

            if (node instanceof JVMDINode) {
                JVMDINode jn = (JVMDINode)node;
                it.remove();
                jn.prune();
                jn.addConstants(addons);
            }
        }
        if (!addons.isEmpty()) {
            components.addAll(addons);
        }
        super.prune();
    }

    void constrainComponent(Context ctx, Node node) {
        if (node instanceof ConstantNode) {
            node.constrain(ctx);
            constantMap.put(name + "_" + ((ConstantNode) node).getName(), node.comment());
        } else {
            error("Expected 'Constant' or 'JVMDI' item, got: " + node);
        }
    }

    void document(PrintWriter writer) {
        writer.println("<h4><a name=\"" + context.whereC + "\">" + name +
                       " Constants</a></h4>");
        writer.println(comment());
        writer.println("<dd><table border=1 cellpadding=3 cellspacing=0 width=\"90%\" summary=\"\"><tr>");
        writer.println("<th width=\"20%\"><th width=\"5%\"><th width=\"65%\">");
        ConstantNode n;
        for (Iterator it = components.iterator(); it.hasNext();) {
            n = ((ConstantNode)it.next());
            writer.println("<a NAME=\"" + name + "_" + n.name + "\"></a>");
            n.document(writer);
        }
        writer.println("</table>");
    }

    void documentIndex(PrintWriter writer) {
        writer.print("<li><a href=\"#" + context.whereC + "\">");
        writer.println(name() + "</a> Constants");
//        writer.println("<ul>");
//        for (Iterator it = components.iterator(); it.hasNext();) {
//            ((Node)it.next()).documentIndex(writer);
//        }
//        writer.println("</ul>");
    }

    void genJavaClassSpecifics(PrintWriter writer, int depth) {
    }

    void genJava(PrintWriter writer, int depth) {
        genJavaClass(writer, depth);
    }
    
    public static String getConstant(String key){
        if (constantMap == null) {
            return "";
        }
        String com = (String) constantMap.get(key);
        if(com == null){
            return "";
        } else {
            return com;
        }
    }

}

