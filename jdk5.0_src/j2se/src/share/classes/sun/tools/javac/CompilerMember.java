/*
 * @(#)CompilerMember.java	1.21 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

/**
 * This class is used to represents fields while they are
 * being compiled
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
@Deprecated
final
class CompilerMember implements Comparable {
    MemberDefinition field;
    Assembler asm;
    Object value;
    String name;
    String sig;
    String key;

    CompilerMember(MemberDefinition field, Assembler asm) {
	this.field = field;
	this.asm = asm;
	name = field.getName().toString();
	sig = field.getType().getTypeSignature();
    }

    public int compareTo(Object o) {
	CompilerMember cm = (CompilerMember) o;
	return getKey().compareTo(cm.getKey());
    }

    String getKey() {
	if (key==null) 
	    key = name+sig;
	return key;
    }

}
