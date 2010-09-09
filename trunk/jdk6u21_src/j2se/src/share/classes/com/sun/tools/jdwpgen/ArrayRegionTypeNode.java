/*
 * @(#)ArrayRegionTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
