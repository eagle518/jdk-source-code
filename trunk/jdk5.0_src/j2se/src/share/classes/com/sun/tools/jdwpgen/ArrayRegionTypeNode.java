/*
 * @(#)ArrayRegionTypeNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class ArrayRegionTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "arrayregion";
    }

    String javaType() {
        return "List";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        error("Not implemented");
    }

    String javaRead() {
        return "ps.readArrayRegion()";
    }
}
