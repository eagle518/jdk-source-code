/*
 * @(#)TypeNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;


import java.util.*;
import java.io.*;

interface TypeNode {

    String name();

    void genJavaWrite(PrintWriter writer, int depth,  String writeLabel);

    void genJavaRead(PrintWriter writer, int depth, String readLabel);

    void genJavaDeclaration(PrintWriter writer, int depth);

    String javaParam();
}
