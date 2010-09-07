/*
 * @(#)MozillaJSSProvider.java	1.1 04/02/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.AccessController;
import java.security.Provider;
import java.security.PrivilegedAction;
import java.security.cert.*;
import java.security.SecureRandom;


/**
 * The "MozillaJSSProvider" Cryptographic Service Provider.
 *
 * Supported algorithms and their names:
 *
 * - Signature NONEwithRSA
 * - Signature NONEwithDSA
 * - Signature SHA1withRSA
 * 
 * - KeyStore "MozillaJSS"
 */

public final class MozillaJSSProvider extends Provider 
{

    private static final String info = "SunDeploy-MozillaJSS Provider (implements RSA)";

    public MozillaJSSProvider() {
	/* We are the "MozillaJSS" provider */
	super("SunDeploy-MozillaJSS", 1.5d, info);

        AccessController.doPrivileged(new java.security.PrivilegedAction() {
            public Object run() 
            {
		/*
		 * Signature engines 
		 */
		put("Signature.NONEwithRSA", "com.sun.deploy.security.MozillaJSSNONEwithRSASignature");
		put("Signature.DSA", "com.sun.deploy.security.MozillaJSSDSASignature$SHA1withDSA");
		put("Signature.RawDSA", "com.sun.deploy.security.MozillaJSSDSASignature$NONEwithDSA");

		/*
		 * KeyStore
		 */
		put("KeyStore.MozillaMy", "com.sun.deploy.security.MozillaMyKeyStore");

		return null;
	    }
	});
    }
}
