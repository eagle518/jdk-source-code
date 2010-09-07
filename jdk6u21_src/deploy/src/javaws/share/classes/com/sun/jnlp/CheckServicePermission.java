/*
 * @(#)CheckServicePermission.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.security.AccessController;
import java.security.AccessControlException;
import java.security.Permission;
import java.security.AllPermission;
import java.io.FilePermission;
import java.awt.AWTPermission;
/*
 *  Class CheckServicePermission
 *  Provide static methods to check whether certain permission is granted.
 *
 */

public final class CheckServicePermission {
    /* Check whether a specific Permission is set for the running thread. This
     * has to be done dynamically - since some parts of an application might 
     * have the permissions - other might not.
     */
    private static boolean checkPermission(Permission p) {
	try {
	    AccessController.checkPermission(p);
	    return true;
	} catch(AccessControlException ace) {
	    return false;
	}
    }
    
    /** Check for File access */
    static boolean hasFileAccessPermissions(String path) {
	return checkPermission(new FilePermission(path, "read,write"));
    }
    
    /** Check for File access */
    static boolean hasFileAccessPermissions() {
	return checkPermission(new FilePermission("*", "read,write"));
    }
    
    /** Check for Print access */
    static boolean hasPrintAccessPermissions() {
	return checkPermission(new RuntimePermission("queuePrintJob"));
    }
    
    /** Check for Clipboard access */
    static boolean hasClipboardPermissions() {
	return checkPermission(new AWTPermission("accessClipboard"));
    }
}

