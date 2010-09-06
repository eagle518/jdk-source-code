/*
 * @(#)FailedDownloadingResourceException.java	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

public class FailedDownloadingResourceException extends DownloadException {
    /** Creates an exception */
    public FailedDownloadingResourceException(LaunchDesc ld, URL location, String versionID, Exception e) {
        super(ld, location, versionID, e);
    }
    
    /** Creates an exception */
    public FailedDownloadingResourceException(URL location, String versionID, Exception e) {
        this(null, location, versionID, e);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return ResourceManager.getString("launch.error.failedloadingresource", getResourceString());
    }
}


