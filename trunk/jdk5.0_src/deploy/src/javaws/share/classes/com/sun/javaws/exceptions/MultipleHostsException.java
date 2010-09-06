/*
 * @(#)MultipleHostsException.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class MultipleHostsException extends JNLPException {
    
    public MultipleHostsException() {
        super(ResourceManager.getString("launch.error.category.security"));
    }
    
    /** Returns message */
    public String getRealMessage() { return ResourceManager.getString("launch.error.multiplehostsreferences");
    }
}


