/*
 * @(#)TypeTree.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

/** Common supertype for all nodes that represent type expressions in
 * the generic signature AST.
 */
public interface TypeTree extends Tree {
    /**
     * Accept method for the visitor pattern.
     * @param v - a <tt>TypeTreeVisitor</tt> that will process this
     * tree
     */
    void accept(TypeTreeVisitor<?> v);
}
