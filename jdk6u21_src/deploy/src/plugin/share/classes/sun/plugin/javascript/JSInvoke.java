/*
 * @(#)JSInvoke.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/*
 * JavaScript to Java invocation trampoline class.
 */
class JSInvoke {
    private static Object invoke(Method m, Object obj, Object[] params)
	throws InvocationTargetException, IllegalAccessException {
	return m.invoke(obj, params);
    }

    private static Object newInstance(Constructor c, Object[] params)
	throws InstantiationException, InvocationTargetException, IllegalAccessException {
	return c.newInstance(params);
    }
}
