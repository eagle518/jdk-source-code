/*
 * @(#)PermissionImpl.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.acl;

import java.security.Principal;
import java.security.acl.*;

/**
 * The PermissionImpl class implements the permission 
 * interface for permissions that are strings.
 * @author Satish Dharmaraj
 */
public class PermissionImpl implements Permission {

    private String permission;

    /**
     * Construct a permission object using a string.
     * @param permission the stringified version of the permission.
     */
    public PermissionImpl(String permission) {
	this.permission = permission;
    }

    /**
     * This function returns true if the object passed matches the permission 
     * represented in this interface.
     * @param another The Permission object to compare with.
     * @return true if the Permission objects are equal, false otherwise
     */
    public boolean equals(Object another) {
	if (another instanceof Permission) {
	    Permission p = (Permission) another;
	    return permission.equals(p.toString());
	} else {
	    return false;
	}
    }
    
    /**
     * Prints a stringified version of the permission.
     * @return the string representation of the Permission.
     */
    public String toString() {
	return permission;
    }

    /**
     * Returns a hashcode for this PermissionImpl.
     *
     * @return a hashcode for this PermissionImpl.
     */
    public int hashCode() {
	return toString().hashCode();
    }

}

