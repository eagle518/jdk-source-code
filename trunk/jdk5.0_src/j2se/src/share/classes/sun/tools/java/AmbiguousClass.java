/*
 * @(#)AmbiguousClass.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This exception is thrown when an unqualified class name
 * is used that can be resolved in more than one way.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */

public
class AmbiguousClass extends ClassNotFound {
    /**
     * The class that was not found
     */
    public Identifier name1;
    public Identifier name2;

    /**
     * Constructor
     */
    public AmbiguousClass(Identifier name1, Identifier name2) {
	super(name1.getName());
	this.name1 = name1;
	this.name2 = name2;
    }
}
