/*
 * @(#)DownloadException.java	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;


/** Root exception for all exceptions that releates
 *  to download resources
 *
 *  It's main thing is to set the category for this
 *  kind of exception.
 */

public class DownloadException extends JNLPException {
    private URL    _location;
    private String _version;
    
    // Temp. message
    private String _message;
    
    /** Creates an exception */
    public DownloadException(URL location, String version) {
        this(null, location, version, null);
    }
     
    /** Creates an exception - that wraps another exception. Should only be called by a subclass, since
     *  no message has been set
     */
    protected DownloadException(LaunchDesc ld, URL location, String version, Exception e) {
        super(ResourceManager.getString("launch.error.category.download"), ld, e);
        _location = location;
        _version = version;
    }
        
    public URL getLocation()   { return _location; }
    public String getVersion() { return _version; }
    
    public String getResourceString() {
        String loc = _location.toString();
        if (_version == null) {
            return ResourceManager.getString("launch.error.resourceID", loc);
        } else {
            return ResourceManager.getString("launch.error.resourceID-version", loc, _version);
        }
    }
    
    /** Returns the message */
    public String getRealMessage() { return _message; }
}



