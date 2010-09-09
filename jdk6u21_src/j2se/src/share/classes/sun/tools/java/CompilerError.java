/*
 * @(#)CompilerError.java	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This exception is thrown when an internal compiler error occurs
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */

public
class CompilerError extends Error {
    Throwable e;

    /**
     * Constructor
     */
    public CompilerError(String msg) {
	super(msg);
	this.e = this;
    }

    /**
     * Create an exception given another exception.
     */
    public CompilerError(Exception e) {
	super(e.getMessage());
	this.e = e;
    }

    public void printStackTrace() {
	if (e == this)
	    super.printStackTrace();
	else
	    e.printStackTrace();
    }
}
