/*
 * @(#)RemainderExpression.java	1.23 03/12/19
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
class RemainderExpression extends DivRemExpression {
    /**
     * constructor
     */
    public RemainderExpression(long where, Expression left, Expression right) {
	super(REM, where, left, right);
    }

    /**
     * Evaluate
     */
    Expression eval(int a, int b) {
	return new IntExpression(where, a % b);
    }
    Expression eval(long a, long b) {
	return new LongExpression(where, a % b);
    }
    Expression eval(float a, float b) {
	return new FloatExpression(where, a % b);
    }
    Expression eval(double a, double b) {
	return new DoubleExpression(where, a % b);
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_irem + type.getTypeCodeOffset());
    }
}
