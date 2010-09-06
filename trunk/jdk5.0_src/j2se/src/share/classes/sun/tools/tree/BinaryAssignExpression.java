/*
 * @(#)BinaryAssignExpression.java	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.util.Hashtable;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public
class BinaryAssignExpression extends BinaryExpression {
    Expression implementation;

    /**
     * Constructor
     */
    BinaryAssignExpression(int op, long where, Expression left, Expression right) {
	super(op, where, left.type, left, right);
    }

    public Expression getImplementation() {
	if (implementation != null)
	    return implementation;
	return this;
    }

    /**
     * Order the expression based on precedence
     */
    public Expression order() {
	if (precedence() >= left.precedence()) {
	    UnaryExpression e = (UnaryExpression)left;
	    left = e.right;
	    e.right = order();
	    return e;
	}
	return this;
    }

    /**
     * Check void expression
     */
    public Vset check(Environment env, Context ctx, Vset vset, Hashtable exp) {
	return checkValue(env, ctx, vset, exp);
    }

    /**
     * Inline
     */
    public Expression inline(Environment env, Context ctx) {
	if (implementation != null)
	    return implementation.inline(env, ctx);
	return inlineValue(env, ctx);
    }
    public Expression inlineValue(Environment env, Context ctx) {
	if (implementation != null)
	    return implementation.inlineValue(env, ctx);
	left = left.inlineLHS(env, ctx);
	right = right.inlineValue(env, ctx);
	return this;
    }

    public Expression copyInline(Context ctx) {
	if (implementation != null)
	    return implementation.copyInline(ctx);
	return super.copyInline(ctx);
    }

    public int costInline(int thresh, Environment env, Context ctx) {
	if (implementation != null)
	    return implementation.costInline(thresh, env, ctx);
	return super.costInline(thresh, env, ctx);
    }
}
