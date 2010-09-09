/*
 * @(#)CommentNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class CommentNode extends AbstractSimpleNode {
    
    String text;

    CommentNode(String text) {
        this.text = text;
    }

    String text() {
        return text;
    }
}
