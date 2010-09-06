/*
 * @(#)ArrayTypeSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

public class ArrayTypeSignature implements FieldTypeSignature {
    private TypeSignature componentType;

    private ArrayTypeSignature(TypeSignature ct) {componentType = ct;}

    public static ArrayTypeSignature make(TypeSignature ct) {
	return new ArrayTypeSignature(ct);
    }

    public TypeSignature getComponentType(){return componentType;}

    public void accept(TypeTreeVisitor<?> v){
	v.visitArrayTypeSignature(this);
    }
}
