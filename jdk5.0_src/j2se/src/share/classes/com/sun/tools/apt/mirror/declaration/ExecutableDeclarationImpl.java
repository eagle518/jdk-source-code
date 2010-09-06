/*
 * @(#)ExecutableDeclarationImpl.java	1.6 04/05/04
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.ReferenceType;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;


/**
 * Implementation of ExecutableDeclaration
 */

public abstract class ExecutableDeclarationImpl extends MemberDeclarationImpl
					     implements ExecutableDeclaration {
    public MethodSymbol sym;

    protected ExecutableDeclarationImpl(AptEnv env, MethodSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns type parameters (if any), method name, and signature
     * (value parameter types).
     */
    public String toString() {
	return sym.toString();
    }

    /**
     * {@inheritDoc}
     */
    public boolean isVarArgs() {
	return AptEnv.hasFlag(sym, Flags.VARARGS);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<ParameterDeclaration> getParameters() {
	Collection<ParameterDeclaration> res =
	    new ArrayList<ParameterDeclaration>();
	for (VarSymbol param : sym.params())
	    res.add(env.declMaker.getParameterDeclaration(param));
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public Collection<ReferenceType> getThrownTypes() {
	ArrayList<ReferenceType> res = new ArrayList<ReferenceType>();
	for (Type t : sym.type.thrown()) {
	    res.add((ReferenceType) env.typeMaker.getType(t));
	}
	return res;
    }
}
