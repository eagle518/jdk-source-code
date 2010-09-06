/*
 * @(#)AllPermissionsImpl.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.acl;

import java.security.Principal;
import java.security.acl.*;

/**
 * This class implements the principal interface for the set of all permissions.
 * @author Satish Dharmaraj
 */
public class AllPermissionsImpl extends PermissionImpl {

    public AllPermissionsImpl(String s) {
	super(s);
    }

    /**
     * This function returns true if the permission passed matches the permission represented in 
     * this interface.
     * @param another The Permission object to compare with.
     * @returns true always
     */
    public boolean equals(Permission another) {
	return true;
    }
}
