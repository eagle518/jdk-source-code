/*
 * @(#)TypeParameterDeclarationImpl.java	1.2 04/05/24
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.ReferenceType;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;


/**
 * Implementation of TypeParameterDeclaration
 */

public class TypeParameterDeclarationImpl extends DeclarationImpl
					  implements TypeParameterDeclaration
{
    protected TypeSymbol sym;


    TypeParameterDeclarationImpl(AptEnv env, TypeSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns the type parameter's name along with any "extends" clause.
     * Class names are qualified.  No implicit "extends Object" is added.
     */
    public String toString() {
	return toString(env, (Type.TypeVar) sym.type);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<ReferenceType> getBounds() {
	ArrayList<ReferenceType> res = new ArrayList<ReferenceType>();
	for (Type t : env.jctypes.getBounds((Type.TypeVar) sym.type)) {
	    res.add((ReferenceType) env.typeMaker.getType(t));
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public Declaration getOwner() {
	Symbol owner = sym.owner;
	return ((owner.kind & Kinds.TYP) != 0)
	       ? env.declMaker.getTypeDeclaration((ClassSymbol) owner)
	       : env.declMaker.getExecutableDeclaration((MethodSymbol) owner);
    }



    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitTypeParameterDeclaration(this);
    }


    /**
     * Returns the type parameter's name along with any "extends" clause.
     * See {@link #toString()} for details.
     */
    static String toString(AptEnv env, Type.TypeVar tv) {
	StringBuilder s = new StringBuilder();
	s.append(tv);
	boolean first = true;
	for (Type bound : getExtendsBounds(env, tv)) {
	    s.append(first ? " extends " : " & ");
	    s.append(env.typeMaker.typeToString(bound));
	    first = false;
	}
	return s.toString();
    }

    /**
     * Returns the bounds of a type variable, eliding java.lang.Object
     * if it appears alone.
     */
    private static Iterable<Type> getExtendsBounds(AptEnv env,
						   Type.TypeVar tv) {
	return (tv.bound().tsym == env.symtab.objectType.tsym)
	       ? Type.emptyList
	       : env.jctypes.getBounds(tv);
    }
}
