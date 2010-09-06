/*
 * @(#)JARSigningException.java	1.9 03/12/19
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

public class JARSigningException extends DownloadException {
    private int    _code;
    private String _missingEntry;
    
    // Error codes
    public static final int MULTIPLE_CERTIFICATES = 0;
    public static final int MULTIPLE_SIGNERS = 1;
    public static final int BAD_SIGNING = 2;
    public static final int UNSIGNED_FILE = 3;
    public static final int MISSING_ENTRY = 4;
    
    public JARSigningException(URL location, String versionID, int code) {
        super(null, location, versionID, null);
        _code = code;
    }

    public JARSigningException(URL location, String versionID, int code, String missingEntry) {
        super(null, location, versionID, null);
        _code = code;
	_missingEntry = missingEntry;
    }
    
    
    /** Creates an exception */
    public JARSigningException(URL location, String versionID, int code, Exception e) {
        super(null, location, versionID, e);
        _code = code;
    }
    
    /** Returns the message */
    public String getRealMessage() {
        switch(_code) {
            case MULTIPLE_CERTIFICATES: return ResourceManager.getString("launch.error.jarsigning-multicerts",   getResourceString());
            case MULTIPLE_SIGNERS:      return ResourceManager.getString("launch.error.jarsigning-multisigners", getResourceString());
            case BAD_SIGNING:           return ResourceManager.getString("launch.error.jarsigning-badsigning",   getResourceString());
            case UNSIGNED_FILE:         return ResourceManager.getString("launch.error.jarsigning-unsignedfile", getResourceString());
	    case MISSING_ENTRY:         return ResourceManager.getString("launch.error.jarsigning-missingentry", getResourceString()) + "\n" + ResourceManager.getString("launch.error.jarsigning-missingentryname", _missingEntry);
        }
        return "<error>";
    }
}


