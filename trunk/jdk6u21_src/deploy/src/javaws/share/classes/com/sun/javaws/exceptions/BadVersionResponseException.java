/*
 * @(#)BadVersionResponseException.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;

/** Root exception for all exceptions that releates
 *  to download resources
 *
 *  It's main thing is to set the category for this
 *  kind of exception.
 */

public class BadVersionResponseException extends DownloadException {
    private String _responseVersionID;
    
    /** Creates an exception */
    public BadVersionResponseException(URL location, String versionID, String responseVersionID) {
        super(location, versionID);
        _responseVersionID = responseVersionID;
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return ResourceManager.getString("launch.error.badversionresponse", getResourceString(), _responseVersionID);
    }
}


