/*
 * @(#)ConstructorDeclarationImpl.java	1.2 04/03/09
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.ArrayList;
import java.util.Collection;

import com.sun.mirror.declaration.*;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Flags;
import com.sun.tools.javac.code.Symbol.MethodSymbol;


/**
 * Implementation of ConstructorDeclaration
 */

public class ConstructorDeclarationImpl extends ExecutableDeclarationImpl
					implements ConstructorDeclaration {

    ConstructorDeclarationImpl(AptEnv env, MethodSymbol sym) {
	super(env, sym);
    }


    /**
     * {@inheritDoc}
     * Returns the simple name of the declaring class.
     */
    public String getSimpleName() {
	return sym.enclClass().name.toString();
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitConstructorDeclaration(this);
    }
}
