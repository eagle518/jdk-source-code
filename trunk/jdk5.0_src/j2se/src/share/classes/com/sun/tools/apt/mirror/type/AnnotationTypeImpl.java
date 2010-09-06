/*
 * @(#)AnnotationTypeImpl.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;


import com.sun.mirror.declaration.*;
import com.sun.mirror.type.*;
import com.sun.mirror.util.TypeVisitor;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.Type;


/**
 * Implementation of AnnotationType
 */

public class AnnotationTypeImpl extends InterfaceTypeImpl
				implements AnnotationType {

    AnnotationTypeImpl(AptEnv env, Type.ClassType type) {
	super(env, type);
    }


    /**
     * {@inheritDoc}
     */
    public AnnotationTypeDeclaration getDeclaration() {
	return (AnnotationTypeDeclaration) super.getDeclaration();
    }

    /**
     * {@inheritDoc}
     */
    public void accept(TypeVisitor v) {
	v.visitAnnotationType(this);
    }
}
