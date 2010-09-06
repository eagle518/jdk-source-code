/*
 * @(#)AppContextUtil.java	1.14 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import sun.awt.SunToolkit;
import sun.awt.AppContext;
import javax.swing.UIManager;
import java.util.Vector;

/* class AppContextUtil - used to factor out depenendencies on SunToolkit */

public class AppContextUtil {

    private static AppContext _mainAppContext = null;
    private static AppContext _securityAppContext = null;

    public static void createSecurityAppContext() {
	
	if (_mainAppContext == null) {
	    _mainAppContext = AppContext.getAppContext();
	}

	if (_securityAppContext == null) {
	    SunToolkit.createNewAppContext();
	    _securityAppContext = AppContext.getAppContext();
	}

    }

    public static boolean isSecurityAppContext() {
        return (AppContext.getAppContext() == _securityAppContext); 
    }

    public static boolean isApplicationAppContext() { 
        return (AppContext.getAppContext() == _mainAppContext); 
    }    
 
}




