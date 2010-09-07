/*
 * @(#)OfflineLaunchException.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class OfflineLaunchException extends JNLPException {
    
    private int type;
    
    public static final int MISSING_RESOURCE = 0;
    public static final int NO_OFFLINE_ALLOWED = 1;
    
    public OfflineLaunchException() {        
        super(ResourceManager.getString("launch.error.category.download"));
    }
    
    public OfflineLaunchException(int i) {
        super(ResourceManager.getString("launch.error.category.download"));
        type = i;
    }
    
    /** Returns message */
    public String getRealMessage() {
        String msg = ResourceManager.getString("launch.error.offlinemissingresource");
        if (type == MISSING_RESOURCE) {
            msg = ResourceManager.getString("launch.error.offlinemissingresource");
        } else if (type == NO_OFFLINE_ALLOWED) {
            msg = ResourceManager.getString("launch.error.offline.noofflineallowed");
        }
        return msg;
    }
}


