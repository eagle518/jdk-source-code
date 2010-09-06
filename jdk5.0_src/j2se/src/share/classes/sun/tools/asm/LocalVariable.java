/*
 * @(#)LocalVariable.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;

/**
 * This class is used to assemble the local variables in the local
 * variable table.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 *
 * @author Arthur van Hoff
 * @version 	1.15, 12/19/03
 */
public final
class LocalVariable {
    MemberDefinition field;
    int slot;
    int from;
    int to;

    public LocalVariable(MemberDefinition field, int slot) {
	if (field == null) {
	    new Exception().printStackTrace();
	}
	this.field = field;
	this.slot = slot;
	to = -1;
    }

    LocalVariable(MemberDefinition field, int slot, int from, int to) {
	this.field = field;
	this.slot = slot;
	this.from = from;
	this.to = to;
    }

    public String toString() {
	return field + "/" + slot;
    }
}
