/*
 * @(#)CacheAccessException.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.exceptions.JNLPException;


public class CacheAccessException extends JNLPException {
    private String  _message;

    /** Construct a CacheAccessException */
    public CacheAccessException(boolean isSystem) {
        super(ResourceManager.getString("launch.error.category.config"));

        if (isSystem) {
            _message = ResourceManager.getString("launch.error.cant.access.system.cache");
        }
        else {
            _message = ResourceManager.getString("launch.error.cant.access.user.cache");
        }
    }

    /** Returns message */
    public String getRealMessage() {
        return (_message);
    }
}
