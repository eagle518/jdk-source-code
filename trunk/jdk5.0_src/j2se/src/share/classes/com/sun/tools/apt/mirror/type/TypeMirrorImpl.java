/*
 * @(#)TypeMirrorImpl.java	1.3 04/04/29
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.type;


import com.sun.mirror.declaration.*;
import com.sun.mirror.type.*;
import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.javac.code.*;


/**
 * Implementation of TypeMirror
 */

public abstract class TypeMirrorImpl implements TypeMirror {

    protected final AptEnv env;
    public final Type type;


    protected TypeMirrorImpl(AptEnv env, Type type) {
	this.env = env;
	this.type = type;
    }


    /**
     * {@inheritDoc}
     */
    public String toString() {
	return type.toString();
    }

    /**
     * {@inheritDoc}
     */
    public boolean equals(Object obj) {
	if (obj instanceof TypeMirrorImpl) {
	    TypeMirrorImpl that = (TypeMirrorImpl) obj;
	    return env.jctypes.isSameType(this.type, that.type);
	} else {
	    return false;
	}
    }

    /**
     * {@inheritDoc}
     */
    public int hashCode() {
	return env.jctypes.hashCode(type);
    }
}
