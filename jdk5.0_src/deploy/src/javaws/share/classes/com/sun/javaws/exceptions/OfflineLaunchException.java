/*
 * @(#)OfflineLaunchException.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class OfflineLaunchException extends JNLPException {
    
    public OfflineLaunchException() {
        super(ResourceManager.getString("launch.error.category.download"));
    }
    
    /** Returns message */
    public String getRealMessage() { return ResourceManager.getString("launch.error.offlinemissingresource");
    }
}


