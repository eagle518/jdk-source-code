/*
 * @(#)BottomSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

public class BottomSignature implements FieldTypeSignature {
    private static BottomSignature singleton = new BottomSignature();

    private BottomSignature(){}

    public static BottomSignature make() {return singleton;}

    public void accept(TypeTreeVisitor<?> v){v.visitBottomSignature(this);}
}
