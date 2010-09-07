/*
 * @(#)DownloadException.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.net.URL;
import java.io.IOException;
import com.sun.deploy.resources.ResourceManager;


/** Root exception for all exceptions that releates
 *  to download resources
 *
 *  It's main thing is to set the category for this
 *  kind of exception.
 */

public class DownloadException extends IOException {
    private URL    _location;
    private String _version;
    private Exception _e;
    // Temp. message
    private String _message;
    
    /** Creates an exception */
    public DownloadException(URL location, String version) {
        this(location, version, null);
    }
     
    /** Creates an exception - that wraps another exception. Should only be called by a subclass, since
     *  no message has been set
     */
    protected DownloadException(URL location, String version, Exception e) {
        super(ResourceManager.getString("launch.error.category.download"));
        _location = location;
        _version = version;
        _e = e;
    }
        
    public URL getLocation()   { return _location; }
    public String getVersion() { return _version; }
    
    public String getResourceString() {
        String loc = _location.toString();
        if (_version == null) {
            return ResourceManager.getString("launch.error.resourceID", loc);
        } else {
            return ResourceManager.getString(
                    "launch.error.resourceID-version", loc, _version);
        }
    }
    
    /** Returns the message */
    public String getRealMessage() { return _message; }
     /** Returns the localized error message for the exception. This is overwritten
     *  to call get getRealMessage to force compile time errors if the subclass does
     *  not implemet it
     */
    public String getMessage() { return getRealMessage(); }
    
    public String getBriefMessage() { return null; }
    
    
    /** Get the expeception that caused this exception to be thrown */
    public Throwable getWrappedException() { return _e ; }
    
    public void printStackTrace() {
        super.printStackTrace();
        if (_e != null) {
            System.err.println("Caused by:");
            _e.printStackTrace();
        }
    }
}



