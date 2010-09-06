/*
 * @(#)ParameterDeclarationImpl.java	1.4 04/04/30
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.TypeMirror;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.VarSymbol;


/**
 * Implementation of ParameterDeclaration
 */

public class ParameterDeclarationImpl extends DeclarationImpl
				      implements ParameterDeclaration
{
    protected VarSymbol sym;


    ParameterDeclarationImpl(AptEnv env, VarSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns the simple name of the parameter.
     */
    public String toString() {
	return getType() + " " + sym.name;
    }

    /**
     * {@inheritDoc}
     */
    public boolean equals(Object obj) {
	// Neither ParameterDeclarationImpl objects nor their symbols
	// are cached by the current implementation, so check symbol
	// owners and names.

	if (obj instanceof ParameterDeclarationImpl) {
	    ParameterDeclarationImpl that = (ParameterDeclarationImpl) obj;
	    return sym.owner == that.sym.owner &&
		   sym.name == that.sym.name &&
		   env == that.env;
	} else {
	    return false;
	}
    }

    /**
     * {@inheritDoc}
     */
    public int hashCode() {
	return sym.owner.hashCode() + sym.name.hashCode() + env.hashCode();
    }

    /**
     * {@inheritDoc}
     */
    public TypeMirror getType() {
	return env.typeMaker.getType(sym.type);
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitParameterDeclaration(this);
    }
}
