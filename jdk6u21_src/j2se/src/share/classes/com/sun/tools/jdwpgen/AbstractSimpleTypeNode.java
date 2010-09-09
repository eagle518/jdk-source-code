/*
 * @(#)AbstractSimpleTypeNode.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

abstract class AbstractSimpleTypeNode extends AbstractTypeNode {

    void constrain(Context ctx) {
        context = ctx;
        nameNode.constrain(ctx);
        if (components.size() != 0) {
            error("Extraneous content: " + components.get(0));
        }
    }
}
