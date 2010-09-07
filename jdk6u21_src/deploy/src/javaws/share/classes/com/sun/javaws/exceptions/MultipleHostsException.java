/*
 * @(#)MultipleHostsException.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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


