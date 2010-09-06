/*
 * @(#)InvalidJarDiffException.java	1.8 03/12/19
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

public class InvalidJarDiffException extends DownloadException {
     
    /** Creates an exception */
    public InvalidJarDiffException(URL location, String versionID, Exception ioe) {
        super(null, location, versionID, ioe);
    }
     
    /** Returns the message */
    public String getRealMessage() {
        return ResourceManager.getString("launch.error.invalidjardiff", getResourceString());
    }
}

