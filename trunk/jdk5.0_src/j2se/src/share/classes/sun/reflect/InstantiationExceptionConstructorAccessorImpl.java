/*
 * @(#)InstantiationExceptionConstructorAccessorImpl.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

/** Throws an InstantiationException with given error message upon
    newInstance() call */

class InstantiationExceptionConstructorAccessorImpl
    extends ConstructorAccessorImpl {
    private String message;

    InstantiationExceptionConstructorAccessorImpl(String message) {
        this.message = message;
    }

    public Object newInstance(Object[] args)
        throws InstantiationException,
               IllegalArgumentException,
               InvocationTargetException
    {
        if (message == null) {
            throw new InstantiationException();
        }
        throw new InstantiationException(message);
    }
}
