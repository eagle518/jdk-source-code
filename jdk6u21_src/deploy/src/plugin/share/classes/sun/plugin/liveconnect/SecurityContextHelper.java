/*
 * @(#)SecurityContextHelper.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.AccessControlException;
import java.security.ProtectionDomain;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.AllPermission;

/**
 * <P> SecurityContextHelper is for evaluating whether an AccessControlContext
 * implies a particular LiveConnect permission.
 * </P>
 *
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */


public class SecurityContextHelper {

    /**
     * <P> Check if permission can be implied with security context.
     * </P>
     *
     * @param context Access Control Context
     * @param target Security target
     * @param action Security action
     * @return true if the operation is successful
     */
    public static boolean Implies(AccessControlContext context, String target, String action)
    {
	if (context == null || target == null)
	    return false;

	try 
	{
	    if (target.equals(LiveConnect.AllJavaPermission))
		context.checkPermission(new AllPermission());
/*	    else if (target.equals(LiveConnect.AllJavaScriptPermission))
		context.checkPermission(new JavaScriptPermission("Binary JavaScript Permission"));
	    else 
		return false;
*/	    else 
		context.checkPermission(new JavaScriptPermission(target));
	}
	catch (AccessControlException e)
	{
	    return false;
	}

	return true;
    }
}
