/*
 * @(#)MSCryptoProvider.java	1.6 04/03/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.AccessController;
import java.security.Provider;
import java.security.PrivilegedAction;
import java.security.cert.*;
import java.security.SecureRandom;


/**
 * The "MSCryptoProvider" Cryptographic Service Provider.
 *
 * Supported algorithms and their names:
 *
 * - Signature NONEwithRSA, RawDSA, SHA1withDSA
 * 
 * - KeyStore "WIExplorer"
 */

public final class MSCryptoProvider extends Provider {

    private static final String info = "SunDeploy-MSCrypto Provider (implements RSA)";

    public MSCryptoProvider() {
	/* We are the "WIExplorer" provider */
	super("SunDeploy-MSCrypto", 1.5d, info);

        AccessController.doPrivileged(new java.security.PrivilegedAction() {
            public Object run() 
            {
		/*
		 * Signature engines 
		 */
		put("Signature.NONEwithRSA", "com.sun.deploy.security.MSCryptoNONEwithRSASignature");
		put("Signature.DSA", "com.sun.deploy.security.MSCryptoDSASignature$SHA1withDSA");
		put("Signature.RawDSA", "com.sun.deploy.security.MSCryptoDSASignature$NONEwithDSA");
		
		/*
		 * KeyStore
		 */
		put("KeyStore.WIExplorerMy", "com.sun.deploy.security.WIExplorerMyKeyStore");

		return null;
	    }
	});
    }
}
