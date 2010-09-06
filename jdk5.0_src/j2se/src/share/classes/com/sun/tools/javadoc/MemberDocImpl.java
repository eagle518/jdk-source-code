/**
 * @(#)MemberDocImpl.java	1.42 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.*;

import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.tree.Tree;

/**
 * Represents a member of a java class: field, constructor, or method.
 * This is an abstract class dealing with information common to
 * method, constructor and field members. Class members of a class
 * (nested classes) are represented instead by ClassDocImpl.
 *
 * @see MethodDocImpl
 * @see FieldDocImpl
 * @see ClassDocImpl
 *
 * @author Robert Field
 * @author Neal Gafter
 */

public abstract class MemberDocImpl
    extends ProgramElementDocImpl
    implements MemberDoc {

    /**
     * constructor.
     */
    public MemberDocImpl(DocEnv env, Symbol sym, String doc, Tree tree) {
        super(env, sym, doc, tree);
    }

    /** 
     * Returns true if this field was synthesized by the compiler.
     */
    public abstract boolean isSynthetic();
}
