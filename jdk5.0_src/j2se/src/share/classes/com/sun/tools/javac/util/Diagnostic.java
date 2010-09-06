/**
 * @(#)Diagnostic.java	1.5 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/** An abstraction of an error message.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Diagnostic {

    public final String key;
    public final Object[] args;

    public Diagnostic(String key, Object ... args) {
	this.key = key;
	this.args = args;
    }

    public String toString() {
	return Log.getLocalizedString(key, args);
    }
}
