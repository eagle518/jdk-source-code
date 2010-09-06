/*
 * @(#)VoidTypeImpl.java	1.1 04/01/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;


import com.sun.mirror.type.VoidType;
import com.sun.mirror.util.TypeVisitor;
import com.sun.tools.apt.mirror.AptEnv;


/**
 * Implementation of VoidType.
 */

class VoidTypeImpl extends TypeMirrorImpl implements VoidType {

    VoidTypeImpl(AptEnv env) {
	super(env, env.symtab.voidType);
    }

    /**
     * {@inheritDoc}
     */
    public void accept(TypeVisitor v) {
	v.visitVoidType(this);
    }
}
