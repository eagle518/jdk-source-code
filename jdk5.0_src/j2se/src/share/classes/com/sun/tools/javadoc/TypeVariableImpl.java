/*
 * @(#)TypeVariableImpl.java	1.4 04/02/06
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

package com.sun.tools.javadoc;


import com.sun.javadoc.*;

import com.sun.tools.javac.code.Kinds;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.code.Type;
import com.sun.tools.javac.code.Type.TypeVar;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.util.Name;

/**
 * Implementation of <code>TypeVariable</code>, which 
 * represents a type variable.
 *
 * @author Scott Seligman
 * @version 1.4 04/02/06
 * @since 1.5
 */
public class TypeVariableImpl extends AbstractTypeImpl implements TypeVariable {

    TypeVariableImpl(DocEnv env, TypeVar type) {
	super(env, type);
    }

    /**
     * Return the bounds of this type variable.
     */
    public com.sun.javadoc.Type[] bounds() {
	return TypeMaker.getTypes(env, getBounds((TypeVar)type, env));
    }

    /**
     * Return the class, interface, method, or constructor within
     * which this type variable is declared.
     */
    public ProgramElementDoc owner() {
 	Symbol osym = type.tsym.owner;
	if ((osym.kind & Kinds.TYP) != 0) {
	    return env.getClassDoc((ClassSymbol)osym);
	}
	Name.Table names = osym.name.table;
	if (osym.name == names.init) {
	    return env.getConstructorDoc((MethodSymbol)osym);
	} else {
	    return env.getMethodDoc((MethodSymbol)osym);
	}
    }

    /**
     * Return the ClassDoc of the erasure of this type variable.
     */
    public ClassDoc asClassDoc() {
	return env.getClassDoc((ClassSymbol)env.types.erasure(type).tsym);
    }

    public TypeVariable asTypeVariable() {
	return this;
    }

    public String toString() {
	return typeVarToString(env, (TypeVar)type, true);
    }


    /**
     * Return the string form of a type variable along with any
     * "extends" clause.  Class names are qualified if "full" is true.
     */
    static String typeVarToString(DocEnv env, TypeVar v, boolean full) {
	StringBuffer s = new StringBuffer(v.toString());
	List<Type> bounds = getBounds(v, env);
	if (bounds.nonEmpty()) {
	    boolean first = true;
	    for (Type b : bounds) {
		s.append(first ? " extends " : " & ");
		s.append(TypeMaker.getTypeString(env, b, full));
		first = false;
	    }
	}
	return s.toString();
    }

    /**
     * Get the bounds of a type variable as listed in the "extends" clause.
     */
    private static List<Type> getBounds(TypeVar v, DocEnv env) {
	Name boundname = v.bound().tsym.fullName();
	if (boundname == boundname.table.java_lang_Object) {
	    return Type.emptyList;
	} else {
	    return env.types.getBounds(v);
	}
    }
}
