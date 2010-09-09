/*
 * @(#)NameNode.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class NameNode extends AbstractSimpleNode {
    
    String name;

    NameNode(String name) {
        this.name = name;
    }

    String text() {
        return name;
    }

    String value() {
        error("Valueless Name asked for value");
        return null;
    }

    public String toString() {
        return name;
    }
}
