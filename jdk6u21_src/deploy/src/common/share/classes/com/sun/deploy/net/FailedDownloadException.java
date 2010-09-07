/*
 * @(#)FailedDownloadException.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.io.IOException;
import java.net.URL;
import com.sun.deploy.resources.ResourceManager;

public class FailedDownloadException extends DownloadException {
    
    private boolean _offline = false;
    
    public FailedDownloadException(URL href, String version,
            Exception e) {        
        super(href, version, e);
    }
    
    public FailedDownloadException(URL href, String version,
            Exception e, boolean offline) {
        super(href, version, e);
        _offline = true;      
    }

    public String getRealMessage() {
        if (_offline) {
            return ResourceManager.getString("launch.error.offline", 
                    getResourceString());
        }
        return ResourceManager.getString("launch.error.failedloadingresource",
                getResourceString());
    }
}
