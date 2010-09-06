/*
 * @(#)DeploySysRun.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

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

    public abstract Object delegate(DeploySysAction action) throws Exception;
}

