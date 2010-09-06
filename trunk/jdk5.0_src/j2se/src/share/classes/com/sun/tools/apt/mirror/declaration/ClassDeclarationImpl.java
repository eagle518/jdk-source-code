/*
 * @(#)ClassDeclarationImpl.java	1.4 04/05/24
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.lang.annotation.Annotation;
import java.lang.annotation.Inherited;
import java.util.ArrayList;
import java.util.Collection;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.ClassType;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;


/**
 * Implementation of ClassDeclaration
 */

public class ClassDeclarationImpl extends TypeDeclarationImpl
				  implements ClassDeclaration {

    ClassDeclarationImpl(AptEnv env, ClassSymbol sym) {
	super(env, sym);
    }


    /**
     * {@inheritDoc}
     * Overridden here to handle @Inherited.
     */
    public <A extends Annotation> A getAnnotation(Class<A> annoType) {

	boolean inherited = annoType.isAnnotationPresent(Inherited.class);
	for (Type t = sym.type;
	     t.tsym != env.symtab.objectType.tsym && !t.isErroneous();
	     t = env.jctypes.supertype(t)) {

	    A result = getAnnotation(annoType, t.tsym);
	    if (result != null || !inherited) {
		return result;
	    }
	}
	return null;
    }

    /**
     * {@inheritDoc}
     */
    public ClassType getSuperclass() {
	//  java.lang.Object has no superclass
	if (sym == env.symtab.objectType.tsym) {
	    return null;
	}
	Type t = env.jctypes.supertype(sym.type);
	return (ClassType) env.typeMaker.getType(t);
    }

    /**
     * {@inheritDoc}
     */
    public Collection<ConstructorDeclaration> getConstructors() {
	ArrayList<ConstructorDeclaration> res =
	    new ArrayList<ConstructorDeclaration>();
	for (Symbol s : getMembers(true)) {
	    if (s.isConstructor()) {
		MethodSymbol m = (MethodSymbol) s;
		res.add((ConstructorDeclaration)
			env.declMaker.getExecutableDeclaration(m));
	    }
	}
	return res;
    }

    /**
     * {@inheritDoc}
     */
    public Collection<MethodDeclaration> getMethods() {
	return identityFilter.filter(super.getMethods(),
				     MethodDeclaration.class);
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitClassDeclaration(this);
    }
}
