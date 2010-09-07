/*
 * @(#)CacheAccessException.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.config.Config;


public class CacheAccessException extends JNLPException {
    private String  _message;

    /** Construct a CacheAccessException */
    public CacheAccessException(boolean isSystem, boolean isDisabled) {
        super(ResourceManager.getString("launch.error.category.config"));

	if (isDisabled) {
	    if (isSystem) {
		_message = ResourceManager.getString(
                    "launch.error.disabled.system.cache");
            } else {
		_message = ResourceManager.getString(
                    "launch.error.disabled.user.cache");
            }
	} else if (Cache.exists()) {

            if (isSystem) {
                _message = ResourceManager.getString(
		    "launch.error.cant.access.system.cache");
            } else {
                _message = ResourceManager.getString(
		    "launch.error.cant.access.user.cache");
	    }
        } else {
	    String type = ResourceManager.getString((isSystem) ?
		"cert.dialog.system.level" : "cert.dialog.user.level");
            if ((isSystem) && (Config.getSystemCacheDirectory() == null)) {
	        _message = 
		    ResourceManager.getString("launch.error.nocache.config");
	    } else {
	        _message = 
		    ResourceManager.getString("launch.error.nocache", type);
	    }
	}
    }

    public CacheAccessException(boolean isSystem) {
	this(isSystem, false);
    }

    /** Returns message */
    public String getRealMessage() {
        return (_message);
    }
}
