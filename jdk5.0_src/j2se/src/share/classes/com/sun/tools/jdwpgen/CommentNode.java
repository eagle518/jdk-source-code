/*
 * @(#)CommentNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
