/*
 * @(#)DeclaredTypeImpl.java	1.4 04/06/07
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;


import java.util.Collection;

import com.sun.mirror.declaration.TypeDeclaration;
import com.sun.mirror.type.*;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.ClassSymbol;


/**
 * Implementation of DeclaredType
 */

abstract class DeclaredTypeImpl extends TypeMirrorImpl
			        implements DeclaredType {

    protected Type.ClassType type;


    protected DeclaredTypeImpl(AptEnv env, Type.ClassType type) {
	super(env, type);
	this.type = type;
    }


    /**
     * Returns a string representation of this declared type.
     * This includes the type's name and any actual type arguments.
     * Type names are qualified.
     */
    public String toString() {
	return toString(env, type);
    }

    /**
     * {@inheritDoc}
     */
    public TypeDeclaration getDeclaration() {
	return env.declMaker.getTypeDeclaration((ClassSymbol) type.tsym);
    }

    /**
     * {@inheritDoc}
     */
    public DeclaredType getContainingType() {
	if (type.outer().tag == TypeTags.CLASS) {
	    // This is the type of an inner class.
	    return (DeclaredType) env.typeMaker.getType(type.outer());
	}
	ClassSymbol enclosing = type.tsym.owner.enclClass();
	if (enclosing != null) {
	    // Nested but not inner.  Return the raw type of the enclosing
	    // class or interface.
	    // See java.lang.reflect.ParameterizedType.getOwnerType().
	    return (DeclaredType) env.typeMaker.getType(
					env.jctypes.erasure(enclosing.type));
	}
	return null;
    }

    /**
     * {@inheritDoc}
     */
    public Collection<TypeMirror> getActualTypeArguments() {
	return env.typeMaker.getTypes(type.typarams());
    }

    /**
     * {@inheritDoc}
     */
    public Collection<InterfaceType> getSuperinterfaces() {
	return env.typeMaker.getTypes(env.jctypes.interfaces(type),
				      InterfaceType.class);
    }


    /**
     * Returns a string representation of this declared type.
     * See {@link #toString()} for details.
     */
    static String toString(AptEnv env, Type.ClassType c) {
	return c.toString();
    }
}
