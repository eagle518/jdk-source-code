/*
 * @(#)JAuthenticator.java	1.33 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import javax.swing.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.jnlp.JNLPClassLoader;
import java.security.AccessControlException;
import com.sun.deploy.security.BrowserAuthenticator;
import com.sun.deploy.security.DeployAuthenticator;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public class JAuthenticator extends com.sun.deploy.security.DeployAuthenticator 
{
    private static JAuthenticator _instance;
    private boolean _challanging = false;
    private boolean _cancel = false;
 
    private JAuthenticator() {
        super();
    }

    static public synchronized JAuthenticator getInstance(Frame f) {
        if (_instance == null) {
            _instance = new JAuthenticator();
        }
	_instance.setParentFrame(f);
        return _instance;
    }
       
    protected synchronized PasswordAuthentication getPasswordAuthentication() 
    {
        PasswordAuthentication pa = null;

	if (Config.getBooleanProperty(Config.SEC_AUTHENTICATOR_KEY)) {

	    _challanging = true;

	    pa = super.getPasswordAuthentication();

	    _challanging = false;
        }
        
        return pa;
    }

    boolean isChallanging() {
	return _challanging;
    }
    
}







