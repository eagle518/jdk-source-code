/**
 * @(#)ParameterImpl.java	1.26 04/05/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.*;

import com.sun.tools.javac.code.Attribute;
import com.sun.tools.javac.code.Symbol.VarSymbol;
import com.sun.tools.javac.code.Type;

/**
 * ParameterImpl information.
 * This includes a parameter type and parameter name.
 *
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 * @author Scott Seligman (generics, annotations)
 */
class ParameterImpl implements Parameter {

    private final DocEnv env;
    private final VarSymbol sym;
    private final com.sun.javadoc.Type type;

    /**
     * Constructor of parameter info class.
     */
    ParameterImpl(DocEnv env, VarSymbol sym) {
	this.env = env;
        this.sym = sym;
        this.type = TypeMaker.getType(env, sym.type, false);
    }

    /**
     * Get the type of this parameter.
     */
    public com.sun.javadoc.Type type() {
        return type;
    }

    /**
     * Get local name of this parameter.
     * For example if parameter is the short 'index', returns "index".
     */
    public String name() {
	return sym.toString();
    }

    /**
     * Get type name of this parameter.
     * For example if parameter is the short 'index', returns "short".
     */
    public String typeName() {
	return (type instanceof ClassDoc || type instanceof TypeVariable)
		? type.typeName()	// omit formal type params or bounds
		: type.toString();
    }

    /**
     * Returns a string representation of the parameter.
     * <p>
     * For example if parameter is the short 'index', returns "short index".
     *
     * @return type name and parameter name of this parameter.
     */
    public String toString() {
	return typeName() + " " + sym;
    }

    /**
     * Get the annotations of this parameter.
     * Return an empty array if there are none.
     */
    public AnnotationDesc[] annotations() {
	AnnotationDesc res[] = new AnnotationDesc[sym.attributes().length()];
	int i = 0;
	for (Attribute.Compound a : sym.attributes()) {
	    res[i++] = new AnnotationDescImpl(env, a);
	}
	return res;
    }
}
