/*
 * @(#)JavawsDialogListener.java	1.4 04/04/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.util;

import com.sun.javaws.SplashScreen;
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
