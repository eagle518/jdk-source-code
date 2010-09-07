/*
 * @(#)DeploySysRun.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;


public abstract class DeploySysRun {

    public static DeploySysRun _subclass;

    public static void setOverride(DeploySysRun subclass) {
	_subclass = subclass;
    }

    public static Object execute(DeploySysAction action) throws Exception {
	if (_subclass != null) {
	    return _subclass.delegate(action);
	} else {
	    Object ret = action.execute();
	    return ret;
	}
    }

    public static Object execute(DeploySysAction action, Object defaultValue) {
	try {
	    return execute(action);
	} catch (Exception e) {
	    Trace.ignoredException(e);
	    return defaultValue;
	}
    }

    public static Object executePrivileged(final DeploySysAction action, 
					   final Object defaultValue) {
        return AccessController.doPrivileged(
            new PrivilegedAction() {
  		public Object run() {
                    try {
                        return DeploySysRun.execute(action);
                    } catch (Exception e) {
                        Trace.ignoredException(e);
                        return defaultValue;
                    }
		}
            });
    }

    public abstract Object delegate(DeploySysAction action) throws Exception;
}

