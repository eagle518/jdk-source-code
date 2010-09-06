/*
 * @(#)FormalTypeParameter.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.tree;

import sun.reflect.generics.visitor.TypeTreeVisitor;

/** AST that represents a formal type parameter. */
public class FormalTypeParameter implements TypeTree {
    private String name;
    private FieldTypeSignature[] bounds;

    private FormalTypeParameter(String n, FieldTypeSignature[] bs) {
        name = n;
	bounds = bs;
    }

    /**
     * Factory method.
     * Returns a formal type parameter with the requested name and bounds.
     * @param n  the name of the type variable to be created by this method.
     * @param bs - the bounds of the type variable to be created by this method.
     * @return a formal type parameter with the requested name and bounds
     */
    public static FormalTypeParameter make(String n, FieldTypeSignature[] bs){
	return new FormalTypeParameter(n,bs);
    }

    public FieldTypeSignature[] getBounds(){return bounds;}
    public String getName(){return name;}

    public void accept(TypeTreeVisitor<?> v){v.visitFormalTypeParameter(this);}
}
