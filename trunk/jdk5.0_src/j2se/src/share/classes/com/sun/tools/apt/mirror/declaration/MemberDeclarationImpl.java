/*
 * @(#)MemberDeclarationImpl.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Type;


/**
 * Implementation of MemberDeclaration
 */

public abstract class MemberDeclarationImpl extends DeclarationImpl
					    implements MemberDeclaration {

    protected MemberDeclarationImpl(AptEnv env, Symbol sym) {
	super(env, sym);
    }


    /**
     * {@inheritDoc}
     */
    public TypeDeclaration getDeclaringType() {
	ClassSymbol c = getDeclaringClassSymbol();
	return (c == null)
	    ? null
	    : env.declMaker.getTypeDeclaration(c);
    }

    /**
     * {@inheritDoc}
     * For methods, constructors, and types.
     */
    public Collection<TypeParameterDeclaration> getFormalTypeParameters() {
	ArrayList<TypeParameterDeclaration> res =
	    new ArrayList<TypeParameterDeclaration>();
	for (Type t : sym.type.typarams()) {
	    res.add(env.declMaker.getTypeParameterDeclaration(t.tsym));
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitMemberDeclaration(this);
    }


    /**
     * Returns the ClassSymbol of the declaring type,
     * or null if this is a top-level type.
     */
    private ClassSymbol getDeclaringClassSymbol() {
	return sym.owner.enclClass();
    }

    /**
     * Returns the formal type parameters of a type, member or constructor
     * as an angle-bracketed string.  Each parameter consists of the simple
     * type variable name and any bounds (with no implicit "extends Object"
     * clause added).  Type names are qualified.
     * Returns "" if there are no type parameters.
     */
    protected static String typeParamsToString(AptEnv env, Symbol sym) {
	if (sym.type.typarams().isEmpty()) {
	    return "";
	}
	StringBuilder s = new StringBuilder();
	for (Type t : sym.type.typarams()) {
	    Type.TypeVar tv = (Type.TypeVar) t;
	    s.append(s.length() == 0 ? "<" : ", ")
	     .append(TypeParameterDeclarationImpl.toString(env, tv));
	}
	s.append(">");
	return s.toString();
    }
}
