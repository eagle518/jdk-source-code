/*
 * @(#)BootstrapConstructorAccessorImpl.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Constructor;

/** Uses Unsafe.allocateObject() to instantiate classes; only used for
    bootstrapping. */

class BootstrapConstructorAccessorImpl extends ConstructorAccessorImpl {
    private Constructor constructor;

    BootstrapConstructorAccessorImpl(Constructor c) {
        this.constructor = c;
    }

    public Object newInstance(Object[] args)
        throws IllegalArgumentException, InvocationTargetException
    {
        try {
            return UnsafeFieldAccessorImpl.unsafe.
                allocateInstance(constructor.getDeclaringClass());
        } catch (InstantiationException e) {
            throw new InvocationTargetException(e);
        }
    }
}
