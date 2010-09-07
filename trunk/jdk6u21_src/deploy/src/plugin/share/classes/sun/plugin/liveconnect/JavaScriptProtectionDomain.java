/*
 * @(#)JavaScriptProtectionDomain.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

import java.security.Permission;
import java.security.PermissionCollection;
import java.security.ProtectionDomain;

/** 
 *
 * <p> This JavaScriptProtectionDomain class encapulates the characteristics of
 * a virtual JavaScript domain, which represents the set of permissions when
 * JavaScript calls Java through LiveConnect.
 * </p>
 * 
 * @author Stanley Man-Kit Ho
 */

public class JavaScriptProtectionDomain extends ProtectionDomain
{
    
    // Permission collection
    PermissionCollection perms = null;

    /**
     * <P> Creates a new JavaScriptProtectionDomain with a given 
     * permission collections.
     * </P>
     *
     * @param securityContext the nsISecurityContext
     */
    public JavaScriptProtectionDomain(PermissionCollection perms) 
    {
	super(null, null);
	this.perms = perms;
    }

    /**
     * <P> Check and see if this JavaScriptProtectionDomain implies the 
     * permissions expressed in the Permission object.
     * </P>
     *
     * @param permission the Permission object to check.
     * @return true if "permission" is a proper subset of a permission in 
     *		    this ProtectionDomain, false if not.
     */
    public boolean implies(Permission permission) 
    {
	return perms.implies(permission);
    }


    /**
     * <P> Convert a JavaScriptProtectionDomain to a String.
     * </P>
     */
    public String toString() {
	return "JavaScriptProtectionDomain "+getCodeSource()+"\n"+getPermissions()+"\n";
    }
}
