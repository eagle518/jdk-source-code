/*
 * %W% %E%
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.cache.Cache;

public class BadFieldException extends LaunchDescException {
    private String  _field;
    private String  _value;
    private String _launchDescSource;  // Reference to JNLP file contents
    
    public BadFieldException(String source, String field, String value) {
	super();
	_value = value;
	_field = field;
	_launchDescSource = source;
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** Returns the value of the offending field */
    public String getValue() { return _value; }
            
    /** Returns message */
    public String getRealMessage() {
        if (Cache.isCacheEnabled() == false) {
            return ResourceManager.getString("launch.error.badfield.nocache");
        } else if (getValue().equals("https")) {
            return ResourceManager.getString("launch.error.badfield", _field, _value) + "\n" + ResourceManager.getString("launch.error.badfield.https");
        } else if (!isSignedLaunchDesc()) {
            return ResourceManager.getString("launch.error.badfield", _field, _value);
        } else {
            return ResourceManager.getString("launch.error.badfield-signedjnlp", _field, _value);
        }
    }
    
    /** Overwrite this to return the source */
    public String getLaunchDescSource() {
	return _launchDescSource;
	
    }
    
    /** toString implementation */
    public String toString() { 
	if (getValue().equals("https")) {
	    return "BadFieldException[ " + getRealMessage() + "]"; 
	}
	return "BadFieldException[ " + getField() + "," + getValue() + "]"; 
    }
}


