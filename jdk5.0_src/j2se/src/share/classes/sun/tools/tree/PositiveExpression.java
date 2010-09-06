/*
 * @(#)PositiveExpression.java	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public
class PositiveExpression extends UnaryExpression {
    /**
     * Constructor
     */
    public PositiveExpression(long where, Expression right) {
	super(POS, where, right.type, right);
    }

    /**
     * Select the type of the expression
     */
    void selectType(Environment env, Context ctx, int tm) {
	if ((tm & TM_DOUBLE) != 0) {
	    type = Type.tDouble;
	} else if ((tm & TM_FLOAT) != 0) {
	    type = Type.tFloat;
	} else if ((tm & TM_LONG) != 0) {
	    type = Type.tLong;
	} else {
	    type = Type.tInt;
	} 
	right = convert(env, ctx, type, right);
    }

    /**
     * Simplify
     */
    Expression simplify() {
	return right;
    }
}
