/**
 * @(#)ConstructorDocImpl.java	1.18 04/05/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.*;

import com.sun.tools.javac.code.Symbol.ClassSymbol;
import com.sun.tools.javac.code.Symbol.MethodSymbol;
import com.sun.tools.javac.tree.Tree.MethodDef;

/**
 * Represents a constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */

public class ConstructorDocImpl
	extends ExecutableMemberDocImpl implements ConstructorDoc {

    /**
     * constructor.
     */
    public ConstructorDocImpl(DocEnv env, MethodSymbol sym) {
	super(env, sym);
    }

    /**
     * constructor.
     */
    public ConstructorDocImpl(DocEnv env, MethodSymbol sym,
			      String docComment, MethodDef tree) {
	super(env, sym, docComment, tree);
    }

    /**
     * Return true if it is a constructor, which it is.
     *
     * @return true
     */
    public boolean isConstructor() {
	return true;
    }

    /**
     * Get the name.
     *
     * @return the name of the member qualified by class (but not package)
     */
    public String name() {
        ClassSymbol c = sym.enclClass();
        String n = c.name.toString();
	for (c = c.owner.enclClass(); c != null; c = c.owner.enclClass()) {
	    n = c.name.toString() + "." + n;
	}
	return n;
    }

    /**
     * Get the name.
     *
     * @return the qualified name of the member.
     */
    public String qualifiedName() {
        return sym.enclClass().fullName().toString();
    }

    /**
     * Returns a string representation of this constructor.  Includes the
     * qualified signature and any type parameters.
     * Type parameters precede the class name, as they do in the syntax
     * for invoking constructors with explicit type parameters using "new".
     * (This is unlike the syntax for invoking methods with explicit type
     * parameters.)
     */
    public String toString() {
	return typeParametersString() + qualifiedName() + signature();
    }
}
