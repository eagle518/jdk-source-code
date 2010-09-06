/*
 * @(#)JSInvoke.java	1.1 04/06/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.invoke;

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
}
