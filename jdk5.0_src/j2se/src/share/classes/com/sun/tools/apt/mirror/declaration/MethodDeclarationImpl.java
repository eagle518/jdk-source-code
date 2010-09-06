/*
 * @(#)MethodDeclarationImpl.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import com.sun.mirror.declaration.*;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.mirror.type.TypeMirror;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Symbol.MethodSymbol;


/**
 * Implementation of MethodDeclaration
 */

public class MethodDeclarationImpl extends ExecutableDeclarationImpl
				   implements MethodDeclaration {

    MethodDeclarationImpl(AptEnv env, MethodSymbol sym) {
	super(env, sym);
    }


    /**
     * {@inheritDoc}
     */
    public TypeMirror getReturnType() {
	return env.typeMaker.getType(sym.type.restype());
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitMethodDeclaration(this);
    }
}
