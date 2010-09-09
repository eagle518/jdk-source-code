/*
 * @(#)InterfaceImplementor.java	1.6 10/03/23 18:06:03
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTAIL. Use is subject to license terms.
 */

package com.sun.script.util;

import javax.script.*;
import java.lang.reflect.*;
import java.security.*;

/*
 * java.lang.reflect.Proxy based interface implementor. This is meant
 * to be used to implement Invocable.getInterface.
 *
 * @version 1.0
 * @author Mike Grogan
 * @since 1.6
 */
public class InterfaceImplementor {
    
    private Invocable engine;
    
    /** Creates a new instance of Invocable */
    public InterfaceImplementor(Invocable engine) {
        this.engine = engine;
    }
    
    private final class InterfaceImplementorInvocationHandler 
                        implements InvocationHandler {
        private Object thiz;
        private AccessControlContext accCtxt;

        public InterfaceImplementorInvocationHandler(Object thiz,
            AccessControlContext accCtxt) {
            this.thiz = thiz;
            this.accCtxt = accCtxt;
        }

        public Object invoke(Object proxy , Method method, Object[] args)
        throws java.lang.Throwable {
            // give chance to convert input args
            args = convertArguments(method, args);
            Object result;
            final Method m = method;
            final Object[] a = args;
            result = AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws Exception {
                    if (thiz == null) {
                        return engine.invokeFunction(m.getName(), a);
                    } else {
                        return engine.invokeMethod(thiz, m.getName(), a);
                    }
                }
            }, accCtxt);
            // give chance to convert the method result
            return convertResult(method, result);
        }
    }
    
    public <T> T getInterface(Object thiz, Class<T> iface)
    throws ScriptException {
        if (iface == null || !iface.isInterface()) {
            throw new IllegalArgumentException("interface Class expected");
        }
        AccessControlContext accCtxt = AccessController.getContext();
        return iface.cast(Proxy.newProxyInstance(iface.getClassLoader(),
            new Class[]{iface},
            new InterfaceImplementorInvocationHandler(thiz, accCtxt)));
    }

    // called to convert method result after invoke
    protected Object convertResult(Method method, Object res) 
                                   throws ScriptException {
        // default is identity conversion
        return res;
    }

    // called to convert method arguments before invoke
    protected Object[] convertArguments(Method method, Object[] args)
                                      throws ScriptException {
        // default is identity conversion
        return args;
    }
}
