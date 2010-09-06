/*
 * @(#)LongSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

/** AST that represents the type long. */
public class LongSignature implements BaseType {
    private static LongSignature singleton = new LongSignature();

    private LongSignature(){}

    public static LongSignature make() {return singleton;}

    public void accept(TypeTreeVisitor<?> v){v.visitLongSignature(this);}
}
