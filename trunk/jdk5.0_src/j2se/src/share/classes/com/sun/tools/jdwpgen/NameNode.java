/*
 * @(#)NameNode.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
