/*
 * @(#)Dispatcher.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
