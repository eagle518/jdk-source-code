/*
 * @(#)CanceledDownloadException.java	1.5 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;

/** An exception indicated that the download was
 *  cancled by the user
 *
 */
public class CanceledDownloadException extends DownloadException {
    /** Creates an exception */
    public CanceledDownloadException(URL location, String versionID) {
        super(location, versionID);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return ResourceManager.getString("launch.error.canceledloadingresource", getResourceString());
    }
}


