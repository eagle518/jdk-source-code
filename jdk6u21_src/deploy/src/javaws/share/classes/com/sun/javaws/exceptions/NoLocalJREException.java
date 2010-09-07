/*
 * @(#)NoLocalJREException.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.jnl.LaunchDesc;


public class NoLocalJREException extends JNLPException {
    private String  _message;

    /** Construct a NoLocalJREException */
    public NoLocalJREException(LaunchDesc ld,
                               String     versionID,
                               boolean    deniedByPrompt) {
        super(ResourceManager.getString("launch.error.category.config"), ld);

        if (deniedByPrompt) {
            _message = ResourceManager.getString("launch.error.wont.download.jre", versionID);
        }
        else {
            _message = ResourceManager.getString("launch.error.cant.download.jre", versionID);
        }
    }

    /** Returns message */
    public String getRealMessage() {
        return (_message);
    }
}
