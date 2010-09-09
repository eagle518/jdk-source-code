/*
 * @(#)ArrayData.java	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public final
class ArrayData {
    Type type;
    int nargs;

    public ArrayData(Type type, int nargs) {
	this.type = type;
	this.nargs = nargs;
    }
}
