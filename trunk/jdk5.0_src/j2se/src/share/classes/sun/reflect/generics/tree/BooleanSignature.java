/*
 * @(#)BooleanSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

/** AST that represents the type boolean. */
public class BooleanSignature implements BaseType {
    private static BooleanSignature singleton = new BooleanSignature();

    private BooleanSignature(){}

    public static BooleanSignature make() {return singleton;}

    public void accept(TypeTreeVisitor<?> v){
	v.visitBooleanSignature(this);
    }
}
