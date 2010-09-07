/*
 * @(#)JAuthenticator.java	1.40 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.net.PasswordAuthentication;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.ComponentRef;

public class JAuthenticator extends com.sun.deploy.security.DeployAuthenticator 
{
    private static JAuthenticator _instance;
    private boolean _challanging = false;
    private boolean _cancel = false;
 
    private JAuthenticator() {
        super();
    }

    static public synchronized JAuthenticator getInstance(ComponentRef c) {
        if (_instance == null) {
            _instance = new JAuthenticator();
        }
	_instance.setParentComponent(c);
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

    public boolean isChallanging() {
        return _challanging;
    }
    
}







