/*
 * @(#)TypeDeclarationImpl.java	1.4 04/05/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.*;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;


/**
 * Implementation of TypeDeclaration
 */

public class TypeDeclarationImpl extends MemberDeclarationImpl
				 implements TypeDeclaration {

    public ClassSymbol sym;


    /**
     * "sym" should be completed before this constructor is called.
     */
    protected TypeDeclarationImpl(AptEnv env, ClassSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns the type's name, with any type parameters (including those
     * of outer classes).  Type names are qualified.
     */
    public String toString() {
	return toString(env, sym);
    }

    /**
     * {@inheritDoc}
     */
    public PackageDeclaration getPackage() {
	return env.declMaker.getPackageDeclaration(sym.packge());
    }

    /**
     * {@inheritDoc}
     */
    public String getQualifiedName() {
	return sym.toString();
    }

    /**
     * {@inheritDoc}
     */
    public Collection<InterfaceType> getSuperinterfaces() {
	return env.typeMaker.getTypes(env.jctypes.interfaces(sym.type),
				      InterfaceType.class);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<FieldDeclaration> getFields() {
	ArrayList<FieldDeclaration> res = new ArrayList<FieldDeclaration>();
	for (Symbol s : getMembers(true)) {
	    if (s.kind == Kinds.VAR) {
		res.add(env.declMaker.getFieldDeclaration((VarSymbol) s));
	    }
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public Collection<? extends MethodDeclaration> getMethods() {
	ArrayList<MethodDeclaration> res = new ArrayList<MethodDeclaration>();
	for (Symbol s : getMembers(true)) {
	    if (s.kind == Kinds.MTH && !s.isConstructor()) {
		MethodSymbol m = (MethodSymbol) s;
		res.add((MethodDeclaration)
			env.declMaker.getExecutableDeclaration(m));
	    }
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public Collection<TypeDeclaration> getNestedTypes() {
	ArrayList<TypeDeclaration> res = new ArrayList<TypeDeclaration>();
	for (Symbol s : getMembers(true)) {
	    if (s.kind == Kinds.TYP) {
		res.add(env.declMaker.getTypeDeclaration((ClassSymbol) s));
	    }
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitTypeDeclaration(this);
    }


    /**
     * Returns a type's name, with any type parameters (including those
     * of outer classes).  Type names are qualified.
     */
    static String toString(AptEnv env, ClassSymbol c) {
	StringBuilder sb = new StringBuilder();
	if (c.isInner()) {
	    // c is an inner class, so include type params of outer.
	    ClassSymbol enclosing = c.owner.enclClass();
	    sb.append(toString(env, enclosing))
	      .append('.')
	      .append(c.name);
	} else {
	    sb.append(c);
	}
	sb.append(typeParamsToString(env, c));
	return sb.toString();
    }
}
