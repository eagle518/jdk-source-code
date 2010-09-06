/*
 * @(#)TypeVariableSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

public class TypeVariableSignature implements FieldTypeSignature {
    private String identifier;

    private TypeVariableSignature(String id) {identifier = id;}


    public static TypeVariableSignature make(String id) {
	return new TypeVariableSignature(id);
    }

    public String getIdentifier(){return identifier;}

    public void accept(TypeTreeVisitor<?> v){
	v.visitTypeVariableSignature(this);
    }
}
