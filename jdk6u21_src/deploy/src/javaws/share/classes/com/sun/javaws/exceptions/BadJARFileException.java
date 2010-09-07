/*
 * @(#)BadJARFileException.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.deploy.resources.ResourceManager;

/**
 *  This is to wrap an exception creating a JAR file. This will typically
 *  wrap a java.util.ZipException
 */

public class BadJARFileException extends DownloadException {
    
    public BadJARFileException (URL location, String version, Exception e) {
        super(null, location, version, e);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return ResourceManager.getString("launch.error.badjarfile", getResourceString());
    }
}

