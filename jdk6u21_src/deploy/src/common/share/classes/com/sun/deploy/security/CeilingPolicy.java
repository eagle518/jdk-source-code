/*
 * @(#)CeilingPolicy.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import java.security.Policy;
import java.security.Security;
import java.security.CodeSource;
import java.security.AccessControlException;
import java.security.PermissionCollection;
import java.security.AllPermission; 
import java.security.Permissions;
import java.security.Permission;
import java.util.Enumeration;


public final class CeilingPolicy {

    private static PermissionCollection _trustedPerms = null;
    private static boolean _initialized = false;

    public static void addTrustedPermissions(PermissionCollection perms) {
	if (!_initialized) {
	    _initialized = true;
	    String url = Config.getProperty(Config.SEC_TRUSTED_POLICY_KEY);
	    if (url != null && url.length() > 0) {
		CodeSource cs = new CodeSource((java.net.URL) null, 
				(java.security.cert.Certificate []) null);

	        int numPolicy = 1;

                while (Security.getProperty(
			"policy.url." + numPolicy) != null) {
                    numPolicy++;
                }
		String key = "policy.url." + numPolicy;

                Security.setProperty(key, url);
		try {
		    Policy policy = new sun.security.provider.PolicyFile();
		    _trustedPerms = policy.getPermissions(cs);
		} catch (Exception e) {
		    Trace.ignoredException(e);
		    // may even be a ClassNotFoundException on PolicyFile
		    // if config insists on ceiling, and we cannot implement 
		    // (as in 1.2.2) give them sandbox only ...
		}
		Security.setProperty(key, "");
	    } else {
		// OK - no property - default to all-permissions
		_trustedPerms = new Permissions();
		_trustedPerms.add(new AllPermission());
	    }
	}
	if (_trustedPerms != null) {
	    Enumeration e = _trustedPerms.elements();
	    while (e.hasMoreElements()) {
		perms.add((Permission)e.nextElement());
	    }
	}
    }

}
