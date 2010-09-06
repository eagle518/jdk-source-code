/*
 * @(#)EventNode.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class EventNode extends ReplyNode {

    void constrain(Context ctx) {
        super.constrain(ctx.inEventSubcontext());
    }
}
