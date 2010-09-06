/*
 * @(#)AmbiguousMember.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

import java.util.Enumeration;

/**
 * This exception is thrown when a field reference is
 * ambiguous.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public
class AmbiguousMember extends Exception {
    /**
     * The field that was not found
     */
    public MemberDefinition field1;
    public MemberDefinition field2;

    /**
     * Constructor
     */
    public AmbiguousMember(MemberDefinition field1, MemberDefinition field2) {
	super(field1.getName() + " + " + field2.getName());
	this.field1 = field1;
	this.field2 = field2;
    }
}
