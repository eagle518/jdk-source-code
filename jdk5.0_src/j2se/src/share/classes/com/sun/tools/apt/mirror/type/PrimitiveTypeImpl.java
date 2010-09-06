/*
 * @(#)PrimitiveTypeImpl.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;



import com.sun.mirror.type.PrimitiveType;
import com.sun.mirror.util.TypeVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Type;

import static com.sun.mirror.type.PrimitiveType.Kind.*;


/**
 * Implementation of PrimitiveType.
 */

class PrimitiveTypeImpl extends TypeMirrorImpl implements PrimitiveType {

    private final Kind kind;	// the kind of primitive


    PrimitiveTypeImpl(AptEnv env, Kind kind) {
	super(env, getType(env, kind));
	this.kind = kind;
    }


    /**
     * {@inheritDoc}
     */
    public Kind getKind() {
	return kind;
    }

    /**
     * {@inheritDoc}
     */
    public void accept(TypeVisitor v) {
	v.visitPrimitiveType(this);
    }


    /**
     * Returns the javac type corresponding to a kind of primitive type.
     */
    private static Type getType(AptEnv env, Kind kind) {
	switch (kind) {
	case BOOLEAN:	return env.symtab.booleanType;
	case BYTE:	return env.symtab.byteType;
	case SHORT:	return env.symtab.shortType;
	case INT:	return env.symtab.intType;
	case LONG:	return env.symtab.longType;
	case CHAR:	return env.symtab.charType;
	case FLOAT:	return env.symtab.floatType;
	case DOUBLE:	return env.symtab.doubleType;
	default:	throw new AssertionError();
	}
    }
}
