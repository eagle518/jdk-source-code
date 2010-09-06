/*
 * @(#)ConstructorAccessorImpl.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.InvocationTargetException;

/** Package-private implementation of the ConstructorAccessor
    interface which has access to all classes and all fields,
    regardless of language restrictions. See MagicAccessorImpl. */

abstract class ConstructorAccessorImpl extends MagicAccessorImpl
    implements ConstructorAccessor {
    /** Matches specification in {@link java.lang.reflect.Constructor} */
    public abstract Object newInstance(Object[] args)
        throws InstantiationException,
               IllegalArgumentException,
               InvocationTargetException;
}
