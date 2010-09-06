/*
 * @(#)AnnotationMirrorImpl.java	1.6 04/07/13
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.LinkedHashMap;
import java.util.Map;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.AnnotationType;
import com.sun.mirror.util.SourcePosition;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Attribute;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.util.Name;
import com.sun.tools.javac.util.Pair;


/**
 * Implementation of AnnotationMirror
 */

public class AnnotationMirrorImpl implements AnnotationMirror {

    protected final AptEnv env;
    protected final Attribute.Compound anno;
    protected final Declaration decl;


    AnnotationMirrorImpl(AptEnv env, Attribute.Compound anno, Declaration decl) {
	this.env = env;
	this.anno = anno;
	this.decl = decl;
    }


    /**
     * Returns a string representation of this annotation.
     * String is of one of the forms:
     *     @com.example.foo(name1=val1, name2=val2)
     *     @com.example.foo(val)
     *     @com.example.foo
     * Omit parens for marker annotations, and omit "value=" when allowed.
     */
    public String toString() {
	StringBuilder sb = new StringBuilder("@");
	Constants.Formatter fmtr = Constants.getFormatter(sb);

	fmtr.append(anno.type.tsym);

	int len = anno.values.length();
	if (len > 0) {		// omit parens for marker annotations
	    sb.append('(');
	    boolean first = true;
	    for (Pair<MethodSymbol, Attribute> val : anno.values) {
		if (!first) {
		    sb.append(", ");
		}
		first = false;

		Name name = val.fst.name;
		if (len > 1 || name != env.names.value) {
		    fmtr.append(name);
		    sb.append('=');
		}
		sb.append(new AnnotationValueImpl(env, val.snd, this));
	    }
	    sb.append(')');
	}
	return fmtr.toString();
    }

    /**
     * {@inheritDoc}
     */
    public AnnotationType getAnnotationType() {
	return (AnnotationType) env.typeMaker.getType(anno.type);
    }

    /**
     * {@inheritDoc}
     */
    public Map<AnnotationTypeElementDeclaration, AnnotationValue>
							getElementValues() {
	Map<AnnotationTypeElementDeclaration, AnnotationValue> res =
	    new LinkedHashMap<AnnotationTypeElementDeclaration,
						   AnnotationValue>(); // whew!
	for (Pair<MethodSymbol, Attribute> val : anno.values) {
	    res.put(getElement(val.fst),
		    new AnnotationValueImpl(env, val.snd, this));
	}
	return res;
    }

    public SourcePosition getPosition() {
	// Return position of the declaration on which this annotation
	// appears.
	return (decl == null) ? null : decl.getPosition();

    }

    public Declaration getDeclaration() {
	return this.decl;
    }

    /**
     * Returns the annotation type element for a symbol.
     */
    private AnnotationTypeElementDeclaration getElement(MethodSymbol m) {
	return (AnnotationTypeElementDeclaration)
		    env.declMaker.getExecutableDeclaration(m);
    }
}
