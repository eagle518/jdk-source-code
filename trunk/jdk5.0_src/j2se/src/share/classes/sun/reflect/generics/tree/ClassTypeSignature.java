/*
 * @(#)ClassTypeSignature.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import java.util.List;
import sun.reflect.generics.visitor.TypeTreeVisitor;


/**
 * AST representing class types.
 */
public class ClassTypeSignature implements FieldTypeSignature {
    private List<SimpleClassTypeSignature> path;


    private ClassTypeSignature(List<SimpleClassTypeSignature> p) {
	path = p;
    }

    public static ClassTypeSignature make(List<SimpleClassTypeSignature> p) {
	return new ClassTypeSignature(p);
    }

    public List<SimpleClassTypeSignature> getPath(){return path;}

    public void accept(TypeTreeVisitor<?> v){v.visitClassTypeSignature(this);}
}
