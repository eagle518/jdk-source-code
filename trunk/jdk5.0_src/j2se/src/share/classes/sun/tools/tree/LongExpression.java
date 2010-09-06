/*
 * @(#)LongExpression.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.io.PrintStream;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public
class LongExpression extends ConstantExpression {
    long value;
    
    /**
     * Constructor
     */
    public LongExpression(long where, long value) {
	super(LONGVAL, where, Type.tLong);
	this.value = value;
    }

    /**
     * Get the value
     */
    public Object getValue() {
	return new Long(value);
    }

    /**
     * Check if the expression is equal to a value
     */
    public boolean equals(int i) {
	return value == i;
    }

    /**
     * Check if the expression is equal to its default static value
     */
    public boolean equalsDefault() {
        return value == 0;
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ldc2_w, new Long(value));
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(value + "L");
    }
}
