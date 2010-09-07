/*
 * @(#)Dispatcher.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.*;

public interface Dispatcher
{
    /**
     * Invoke a method according to the method index.
     *
     * @param obj wrapped object
     * @param args Arguments.
     * @return Java object.
     */
    public Object invoke(Object obj, Object []parameters)
        throws  Exception;

    /**
     * Returns the type of return value
     *
     * @return Java object.
     */
    public Class getReturnType();
}
