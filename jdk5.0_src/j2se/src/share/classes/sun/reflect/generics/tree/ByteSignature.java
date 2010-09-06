/*
 * @(#)ByteSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

/** AST that represents the type byte. */
public class ByteSignature implements BaseType {
    private static ByteSignature singleton = new ByteSignature();

    private ByteSignature(){}

    public static ByteSignature make() {return singleton;}

    public void accept(TypeTreeVisitor<?> v){
	v.visitByteSignature(this);
    }
}
