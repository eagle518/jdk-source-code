/*
 * @(#)FieldDeclarationImpl.java	1.3 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.TypeMirror;
import com.sun.mirror.util.DeclarationVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.code.TypeTags;


/**
 * Implementation of FieldDeclaration
 */

class FieldDeclarationImpl extends MemberDeclarationImpl
				  implements FieldDeclaration {

    protected VarSymbol sym;

    FieldDeclarationImpl(AptEnv env, VarSymbol sym) {
	super(env, sym);
	this.sym = sym;
    }


    /**
     * Returns the field's name.
     */
    public String toString() {
	return getSimpleName();
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
    public Object getConstantValue() {
	// If this is a constant, ensure that the initializer has been
	// evaluated.
	env.attr.evalInit(sym);
	Object val = sym.constValue;
	// val may be null, indicating that this is not a constant.

	return Constants.decodeConstant(val, sym.type);
    }

    /**
     * {@inheritDoc}
     */
    public String getConstantExpression() {
	Object val = getConstantValue();
	if (val == null) {
	    return null;
	}
	Constants.Formatter fmtr = Constants.getFormatter();
	fmtr.append(val);
	return fmtr.toString();
    }

    /**
     * {@inheritDoc}
     */
    public void accept(DeclarationVisitor v) {
	v.visitFieldDeclaration(this);
    }
}
