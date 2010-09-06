/**
 * @(#)FatalError.java	1.12 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/** Throwing an instance of this class causes immediate termination
 *  of the main compiler method.  It is used when some non-recoverable
 *  error has been detected in the compiler environment at runtime.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class FatalError extends Error {
    private static final long serialVersionUID = 0;

    /** Construct a <code>FatalError</code> with no detail message.
     */
    public FatalError() {
	super();
    }

    /** Construct a <code>FatalError</code> with the specified detail message.
     *  @param d A diagnostic containing the reason for failure.
     */
    public FatalError(Diagnostic d) {
	super(d.toString());
    }

    /** Construct a <code>FatalError</code> with the specified detail message.
     *  @param s An English(!) string describing the failure, typically because
     *		 the diagnostic resources are missing.
     */
    public FatalError(String s) {
	super(s);
    }
}


