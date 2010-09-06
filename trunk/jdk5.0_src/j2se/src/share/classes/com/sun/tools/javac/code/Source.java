/*
 * @(#)Source.java	1.33 04/05/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

import com.sun.tools.javac.util.*;
import com.sun.tools.javac.jvm.Target;
import java.util.*;

/** The source language version accepted.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public enum Source {
    /** 1.0 had no inner classes, and so could not pass the JCK. */
    // public static final Source JDK1_0 =		new Source("1.0");

    /** 1.1 did not have strictfp, and so could not pass the JCK. */
    // public static final Source JDK1_1 =		new Source("1.1");

    /** 1.2 introduced strictfp. */
    JDK1_2("1.2"),

    /** 1.3 is the same language as 1.2. */
    JDK1_3("1.3"),

    /** 1.4 introduced assert. */
    JDK1_4("1.4"),

    /** 1.5 introduced generics, attributes, foreach, boxing, static import,
     *  covariant return, enums, varargs, et al. */
    JDK1_5("1.5");

    protected static final Context.Key<Source> sourceKey
	= new Context.Key<Source>();

    public static Source instance(Context context) {
	Source instance = context.get(sourceKey);
	if (instance == null) {
	    Options options = Options.instance(context);
	    String sourceString = options.get("-source");
	    if (sourceString != null) instance = lookup(sourceString);
	    if (instance == null) instance = DEFAULT;
	    context.put(sourceKey, instance);
	}
	return instance;
    }

    public final String name;

    private static Map<String,Source> tab = new HashMap<String,Source>();
    static {
	for (Source s : values()) {
	    tab.put(s.name, s);
	}
	tab.put("5", JDK1_5); // Make 5 an alias for 1.5
    }

    private Source(String name) {
	this.name = name;
    }

    public static final Source DEFAULT = JDK1_5;

    public static Source lookup(String name) {
	return tab.get(name);
    }

    public Target requiredTarget() {
	if (this.compareTo(JDK1_5) >= 0) return Target.JDK1_5;
	if (this.compareTo(JDK1_4) >= 0) return Target.JDK1_4;
	return Target.JDK1_1;
    }

    /** Allow encoding errors, giving only warnings. */
    public boolean allowEncodingErrors() {
	return true;
    }
    public boolean allowAsserts() {
	return compareTo(JDK1_4) >= 0;
    }
    public boolean allowCovariantReturns() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowGenerics() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowEnums() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowForeach() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowStaticImport() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowBoxing() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowVarargs() {
	return compareTo(JDK1_5) >= 0;
    }
    public boolean allowAnnotations() {
	return compareTo(JDK1_5) >= 0;
    }
    // hex floating-point literals supported?
    public boolean allowHexFloats() {
	return compareTo(JDK1_5) >= 0;
    }
}
