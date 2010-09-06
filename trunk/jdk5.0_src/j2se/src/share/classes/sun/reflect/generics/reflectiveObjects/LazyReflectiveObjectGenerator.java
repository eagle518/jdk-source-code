/*
 * @(#)LazyReflectiveObjectGenerator.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.generics.reflectiveObjects;

import sun.reflect.generics.factory.GenericsFactory;
import sun.reflect.generics.visitor.Reifier;


/**
 * Common infrastructure for things that lazily generate reflective generics 
 * objects.
 * <p> In all these cases, one needs produce a visitor that will, on demand,
 * traverse the stored AST(s) and reify them into reflective objects.
 * The visitor needs to be initialized with a factory, which will be 
 * provided when the instance is initialized.
 * The factory should be cached.
 * 
*/
public abstract class LazyReflectiveObjectGenerator {
    private GenericsFactory factory; // cached factory

    protected LazyReflectiveObjectGenerator(GenericsFactory f) {
	factory = f;
    }

    // accessor for factory
    private GenericsFactory getFactory() { 
	return factory;
    }

    // produce a reifying visitor (could this be typed as a TypeTreeVisitor?
    protected Reifier getReifier(){return Reifier.make(getFactory());}

}
