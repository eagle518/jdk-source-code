/*
 * @(#)AbstractTypeImpl.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

package com.sun.tools.javadoc;


import com.sun.javadoc.*;

import com.sun.tools.javac.code.Type;


/**
 * Abstract implementation of <code>Type</code>, with useful
 * defaults for the methods in <code>Type</code> (and a couple from
 * <code>ProgramElementDoc</code>).
 *
 * @author Scott Seligman
 * @version 1.3 03/12/19
 * @since 1.5
 */
abstract class AbstractTypeImpl implements com.sun.javadoc.Type {

    protected final DocEnv env;
    protected final Type type;

    protected AbstractTypeImpl(DocEnv env, Type type) {
	this.env = env;
	this.type = type;
    }

    public String typeName() {
	return type.tsym.name.toString();
    }

    public String qualifiedTypeName() {
	return type.tsym.fullName().toString();
    }

    public String simpleTypeName() {
	return type.tsym.name.toString();
    }

    public String name() {
	return typeName();
    }

    public String qualifiedName() {
	return qualifiedTypeName();
    }

    public String toString() {
	return qualifiedTypeName();
    }

    public String dimension() {
	return "";
    }

    public boolean isPrimitive() {
	return false;
    }

    public ClassDoc asClassDoc() {
	return null;
    }

    public TypeVariable asTypeVariable() {
	return null;
    }

    public WildcardType asWildcardType() {
	return null;
    }

    public ParameterizedType asParameterizedType() {
	return null;
    }

    public AnnotationTypeDoc asAnnotationTypeDoc() {
	return null;
    }
}
