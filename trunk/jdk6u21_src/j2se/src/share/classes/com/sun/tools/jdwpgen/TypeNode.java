/*
 * @(#)TypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
