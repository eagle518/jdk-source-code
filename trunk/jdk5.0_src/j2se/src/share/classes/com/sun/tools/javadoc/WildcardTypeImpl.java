/*
 * @(#)WildcardTypeImpl.java	1.8 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

package com.sun.tools.javadoc;


import com.sun.javadoc.*;

import static com.sun.javadoc.LanguageVersion.*;

import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Type.ArgumentType;
import com.sun.tools.javac.util.List;


/**
 * Implementation of <code>WildcardType</code>, which 
 * represents a wildcard type.
 *
 * @author Scott Seligman
 * @version 1.8 04/04/20
 * @since 1.5
 */
public class WildcardTypeImpl extends AbstractTypeImpl implements WildcardType {

    WildcardTypeImpl(DocEnv env, ArgumentType type) {
	super(env, type);
    }

    /**
     * Return the upper bounds of this wildcard type argument
     * as given by the <i>extends</i> clause.
     * Return an empty array if no such bounds are explicitly given.
     */
    public com.sun.javadoc.Type[] extendsBounds() {
	return TypeMaker.getTypes(env, getExtendsBounds((ArgumentType)type));
    }

    /**
     * Return the lower bounds of this wildcard type argument
     * as given by the <i>super</i> clause.
     * Return an empty array if no such bounds are explicitly given.
     */
    public com.sun.javadoc.Type[] superBounds() {
	return TypeMaker.getTypes(env, getSuperBounds((ArgumentType)type));
    }

    /**
     * Return the ClassDoc of the erasure of this wildcard type.
     */
    public ClassDoc asClassDoc() {
	return env.getClassDoc((ClassSymbol)env.types.erasure(type).tsym);
    }

    public WildcardType asWildcardType() {
	return this;
    }

    public String typeName()		{ return "?"; }
    public String qualifiedTypeName()	{ return "?"; }
    public String simpleTypeName()	{ return "?"; }

    public String toString() {
	return wildcardTypeToString(env, (ArgumentType)type, true);
    }


    /**
     * Return the string form of a wildcard type ("?") along with any
     * "extends" or "super" clause.  Delimiting brackets are not
     * included.  Class names are qualified if "full" is true.
     */
    static String wildcardTypeToString(DocEnv env,
				       ArgumentType wildThing, boolean full) {
	if (env.legacyDoclet) {
	    return TypeMaker.getTypeName(env.types.erasure(wildThing), full);
	}
	StringBuffer s = new StringBuffer("?");
	List<Type> bounds = getExtendsBounds(wildThing);
	if (bounds.nonEmpty()) {
	    s.append(" extends ");
	} else {
	    bounds = getSuperBounds(wildThing);
	    if (bounds.nonEmpty()) {
		s.append(" super ");
	    }
	}
	boolean first = true;	// currently only one bound is allowed
	for (Type b : bounds) {
	    if (!first) {
		s.append(" & ");
	    }
	    s.append(TypeMaker.getTypeString(env, b, full));
	    first = false;
	}
	return s.toString();
    }

    private static List<Type> getExtendsBounds(ArgumentType wild) {
	return wild.isSuperBound()
		? Type.emptyList
		: List.make(wild.type);
    }

    private static List<Type> getSuperBounds(ArgumentType wild) {
	return wild.isExtendsBound()
		? Type.emptyList
		: List.make(wild.type);
    }
}
