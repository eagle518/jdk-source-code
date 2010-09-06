/*
 * @(#)PutAllAction.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.action;

import java.util.Map;

import java.security.Provider;
import java.security.PrivilegedAction;

/**
 * A convenience PrivilegedAction class for setting the properties of
 * a provider. See the SunRsaSign provider for a usage example.
 *
 * @see sun.security.rsa.SunRsaSign
 * @author  Andreas Sterbenz
 * @version 1.2, 12/19/03
 * @since   1.5
 */
public class PutAllAction implements PrivilegedAction {
    
    private final Provider provider;
    private final Map map;
    
    public PutAllAction(Provider provider, Map map) {
	this.provider = provider;
	this.map = map;
    }
    
    public Object run() {
	provider.putAll(map);
	return null;
    }
    
}
