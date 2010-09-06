/**
 * @(#)Options.java	1.9 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

import java.util.*;

/** A table of all command-line options.
 *  If an option has an argument, the option name is mapped to the argument.
 *  If a set option has no argument, it is mapped to itself.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Options extends HashMap<String,String> {
    private static final long serialVersionUID = 0;

    /** The context key for the options. */
    protected static final Context.Key<Options> optionsKey =
	new Context.Key<Options>();

    /** Get the Options instance for this context. */
    public static Options instance(Context context) {
	Options instance = context.get(optionsKey);
	if (instance == null)
	    instance = new Options(context);
	return instance;
    }

    protected Options(Context context) {
	super();
	context.put(optionsKey, this);
    }

    static final String LINT = "-Xlint";

    /** Check for a lint suboption. */
    public boolean lint(String s) {
	// return true if either the specific option is enabled, or
	// they are all enabled without the specific one being
	// disabled
	return
	    get(LINT + ":" + s)!=null ||
	    (get(LINT)!=null || get(LINT + ":all")!=null) &&
	        get(LINT+":-"+s)==null;
    }
}
