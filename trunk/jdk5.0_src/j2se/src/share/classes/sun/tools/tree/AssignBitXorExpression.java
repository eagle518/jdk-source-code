/*
 * @(#)AssignBitXorExpression.java	1.20 03/12/19
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
class AssignBitXorExpression extends AssignOpExpression {
    /**
     * Constructor
     */
    public AssignBitXorExpression(long where, Expression left, Expression right) {
	super(ASGBITXOR, where, left, right);
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ixor + itype.getTypeCodeOffset());
    }
}
