/*
 * @(#)ApiDialog.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.security.*;
import java.util.HashSet;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;


/** Package private */
public final class ApiDialog {
    private boolean _remembered = false;
    private boolean _answer = false;
    private boolean _cbChecked = false;
    private String _initMessage = null;

    public ApiDialog() {
    }
    
    boolean askUser(String title, String message, String always, 
		    String label, String contents, boolean alwaysChecked) {
    
	if (!Config.getBooleanProperty(Config.SEC_SANDBOX_JNLP_ENHANCED_KEY)) {
	    // user not allowed to answer such questions
	    return false;
	}

        if (_remembered) {
            return _answer;
        }
	AppInfo ainfo = 
	    JNLPClassLoaderUtil.getInstance().getLaunchDesc().getAppInfo();

	int result = UIFactory.showApiDialog(null, ainfo, title, message,
			label, contents, always, alwaysChecked);


	if (result == UIFactory.ALWAYS) {
	    _remembered = true;
	    _answer = true;
	} else {
	    _answer = (result == UIFactory.OK);
	}
        return _answer;
    }

    boolean askUser(String title, String message, String always) {
	return askUser(title, message, always, null, null, false);
    }

    private HashSet _connect = new HashSet();
    private HashSet _connectNo = new HashSet();

    public boolean askConnect(String host) {
  	
	if (_connect.contains(host)) {
	    return true;
	}
	if (_connectNo.contains(host)) {
	    return false;
	}
	String title = ResourceManager.getString("api.ask.host.title");
        String s = ResourceManager.getString("api.ask.connect", host);
	if (askUser(title, s, null)) {
	    _connect.add(host);
	    // store both hostname and ip to avoid twice certifications.
	    // see bug :in.addr-arpa lookups are not necessary  
	    try {
	        // Because "InetAddress.getByName(host)" will call "askConnect",
	        // it must be call after host being added into "_connect". 
	        String address = InetAddress.getByName(host).getHostAddress();
	        _connect.add(address);
	    } catch (UnknownHostException e) {
	    	Trace.ignored(e);
	    }
	    return true;
	}
	_connectNo.add(host);
	return false;
    }

    private HashSet _accept = new HashSet();
    private HashSet _acceptNo = new HashSet();

    public boolean askAccept(String host) {
        // Lookup host firstly to reduce a call of "getHostAddress()" 
        // if host has existed in cache
    	if (_accept.contains(host)) {
    	    return true;
    	}
    	if (_acceptNo.contains(host)) {
	    return false;
	}
    	try {
    	    host = InetAddress.getByName(host).getHostAddress();
    	} catch (UnknownHostException e) {
	    Trace.ignored(e);
	}
	if (_accept.contains(host)) {
	    return true;
	}
	if (_acceptNo.contains(host)) {
	    return false;
	}
	String title = ResourceManager.getString("api.ask.host.title");
	String s = ResourceManager.getString("api.ask.accept", host);
	if (askUser(title, s, null)) {
	    _accept.add(host);
	    return true;
	}
	_acceptNo.add(host);
	return false;
    }    

}

