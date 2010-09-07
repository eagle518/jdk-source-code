/*
 * @(#)ErrorCodeResponseException.java	1.14 10/03/24
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

public class ErrorCodeResponseException extends DownloadException {
    private String  _errorLine;
    private int     _errorCode;
    private boolean _jreDownload;
    
    // Error codes
    public static final int ERR_10_NO_RESOURCE  = 10; // Could not locate resource
    public static final int ERR_11_NO_VERSION   = 11; // Could not locate requested version
    public static final int ERR_20_UNSUP_OS     = 20; // Unsupported operaton system
    public static final int ERR_21_UNSUP_ARCH   = 21; // Unsupported architecture
    public static final int ERR_22_UNSUP_LOCALE = 22; // Unsupported locale
    public static final int ERR_23_UNSUP_JRE    = 23; // Unsupported JRE version
    public static final int ERR_99_UNKNOWN      = 99; // Unknown error
    
    /** Creates an exception */
    public ErrorCodeResponseException(URL location, String versionID, String errorLine) {
        super(location, versionID);
        _errorLine = errorLine;
        _jreDownload = false;
        
        /** Parse error line */
        _errorCode = ERR_99_UNKNOWN;
        if (_errorLine != null) {
            try {
                int idx = _errorLine.indexOf(' ');
                if (idx != -1) _errorCode = Integer.parseInt(_errorLine.substring(0, idx));
            } catch(NumberFormatException e) {
                _errorCode = ERR_99_UNKNOWN;
            }
        }
    }
    
    public void setJreDownload(boolean flag) { _jreDownload = flag; }
    
    /** Return error code */
    public int getErrorCode() { return _errorCode; }
    
    /** Returns the message */
    public String getRealMessage() {
        String msg = (_jreDownload) ? ResourceManager.getString("launch.error.noJre") : "";
        if (_errorCode != ERR_99_UNKNOWN) {
            return msg + ResourceManager.getString("launch.error.errorcoderesponse-known", getResourceString(), _errorCode, _errorLine);
        } else {
            return msg + ResourceManager.getString("launch.error.errorcoderesponse-unknown", getResourceString());
        }
    }
}




