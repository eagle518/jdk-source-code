/*
 * @(#)AnnotationValueImpl.java	1.7 04/07/14
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.declaration;


import java.util.Collection;
import java.util.ArrayList;

import com.sun.mirror.declaration.*;
import com.sun.mirror.util.SourcePosition;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Attribute;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.code.TypeTags;


/**
 * Implementation of AnnotationValue
 */

public class AnnotationValueImpl implements AnnotationValue {

    protected final AptEnv env;
    protected final Attribute attr;
    protected final AnnotationMirrorImpl annotation;

    AnnotationValueImpl(AptEnv env, Attribute attr, AnnotationMirrorImpl annotation) {
	this.env = env;
	this.attr = attr;
	this.annotation = annotation;
    }


    /**
     * {@inheritDoc}
     */
    public String toString() {
	StringBuilder sb = new StringBuilder();
	Constants.Formatter fmtr = Constants.getFormatter(sb);

	fmtr.append(getValue());
	return fmtr.toString();
    }

    /**
     * {@inheritDoc}
     */
    public Object getValue() {
	ValueVisitor vv = new ValueVisitor();
	attr.accept(vv);
	return vv.value;
    }


    public SourcePosition getPosition() {
	// Imprecise implementation; just return position of enclosing
	// annotation.
	return (annotation == null) ? null : annotation.getPosition();
    }

    private class ValueVisitor implements Attribute.Visitor {

	public Object value;
	
	public void visitConstant(Attribute.Constant c) {
	    value = Constants.decodeConstant(c.value, c.type);
	}

	public void visitClass(Attribute.Class c) {
	    value = env.typeMaker.getType(
			env.jctypes.erasure(c.type));
	}

	public void visitEnum(Attribute.Enum e) {
	    value = env.declMaker.getFieldDeclaration(e.value);
	}

	public void visitCompound(Attribute.Compound c) {
	    value = new AnnotationMirrorImpl(env, c,
					     (annotation == null) ?
					     null :
					     annotation.getDeclaration());
	}

	public void visitArray(Attribute.Array a) {
	    ArrayList<AnnotationValue> vals =
		new ArrayList<AnnotationValue>(a.values.length);
	    for (Attribute elem : a.values) {
		vals.add(new AnnotationValueImpl(env, elem, annotation));
	    }
	    value = vals;
	}

	public void visitError(Attribute.Error e) {
	    value = "<error>";	// javac will already have logged an error msg
	}
    }
}
