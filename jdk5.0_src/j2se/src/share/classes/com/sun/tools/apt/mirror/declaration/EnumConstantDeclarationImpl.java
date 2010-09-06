/*
 * @(#)EnumConstantDeclarationImpl.java	1.2 04/03/09
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import com.sun.mirror.declaration.*;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Symbol.VarSymbol;


/**
 * Implementation of EnumConstantDeclaration
 */

public class EnumConstantDeclarationImpl extends FieldDeclarationImpl
					 implements EnumConstantDeclaration {

    EnumConstantDeclarationImpl(AptEnv env, VarSymbol sym) {
	super(env, sym);
    }

    /**
     * {@inheritDoc}
     */
    public EnumDeclaration getDeclaringType() {
	return (EnumDeclaration) super.getDeclaringType();
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitEnumConstantDeclaration(this);
    }
}
