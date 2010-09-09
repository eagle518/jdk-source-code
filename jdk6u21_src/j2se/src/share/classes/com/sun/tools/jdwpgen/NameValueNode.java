/*
 * @(#)NameValueNode.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class NameValueNode extends NameNode {
    
    String val;

    NameValueNode(String name, String val) {
        super(name);
        this.val = val;
    }

    NameValueNode(String name, int ival) {
        super(name);
        this.val = Integer.toString(ival);
    }

    String value() {
        return val;
    }

    public String toString() {
        return name + "=" + val;
    }
}
