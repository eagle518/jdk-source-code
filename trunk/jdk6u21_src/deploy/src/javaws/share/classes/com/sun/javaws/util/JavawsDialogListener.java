/*
 * @(#)JavawsDialogListener.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.util;

import com.sun.javaws.ui.SplashScreen;
import com.sun.deploy.util.DialogListener;
import java.security.AccessController;
import java.security.PrivilegedAction;

public final class JavawsDialogListener implements DialogListener {

    public JavawsDialogListener() {};

    public void beforeShow() {

	  AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run(){
		SplashScreen.hide();
		return null;
            }
	  });
    }
}
