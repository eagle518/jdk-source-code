/*
 * @(#)SunRsaSign.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.rsa;

import java.util.*;

import java.security.*;

import sun.security.action.PutAllAction;

/**
 * Provider class for the RSA signature provider. Supports RSA keyfactory,
 * keypair generation, and RSA signatures.
 *
 * @since   1.5
 * @version 1.3, 12/19/03
 * @author  Andreas Sterbenz
 */
public final class SunRsaSign extends Provider {
    
    private static final long serialVersionUID = 866040293550393045L;
    
    public SunRsaSign() {
	super("SunRsaSign", 1.5d, "Sun RSA signature provider");

	// if there is no security manager installed, put directly into
	// the provider. Otherwise, create a temporary map and use a
	// doPrivileged() call at the end to transfer the contents
	final Map map = (System.getSecurityManager() == null)
		      ? (Map)this : new HashMap();
	
	// main algorithms

	map.put("KeyFactory.RSA", 
		"sun.security.rsa.RSAKeyFactory");
	map.put("KeyPairGenerator.RSA", 
		"sun.security.rsa.RSAKeyPairGenerator");
	map.put("Signature.MD2withRSA", 
		"sun.security.rsa.RSASignature$MD2withRSA");
	map.put("Signature.MD5withRSA",
		"sun.security.rsa.RSASignature$MD5withRSA");
	map.put("Signature.SHA1withRSA", 
		"sun.security.rsa.RSASignature$SHA1withRSA");
	map.put("Signature.SHA256withRSA", 
		"sun.security.rsa.RSASignature$SHA256withRSA");
	map.put("Signature.SHA384withRSA",
		"sun.security.rsa.RSASignature$SHA384withRSA");
	map.put("Signature.SHA512withRSA", 
		"sun.security.rsa.RSASignature$SHA512withRSA");
	
	// attributes for supported key classes
	
	String rsaKeyClasses = "java.security.interfaces.RSAPublicKey" +
		"|java.security.interfaces.RSAPrivateKey";
	map.put("Signature.MD2withRSA SupportedKeyClasses", rsaKeyClasses);
	map.put("Signature.MD5withRSA SupportedKeyClasses", rsaKeyClasses);
	map.put("Signature.SHA1withRSA SupportedKeyClasses", rsaKeyClasses);
	map.put("Signature.SHA256withRSA SupportedKeyClasses", rsaKeyClasses);
	map.put("Signature.SHA384withRSA SupportedKeyClasses", rsaKeyClasses);
	map.put("Signature.SHA512withRSA SupportedKeyClasses", rsaKeyClasses);
    
	// aliases
	
	map.put("Alg.Alias.KeyFactory.1.2.840.113549.1.1",     "RSA");
	map.put("Alg.Alias.KeyFactory.OID.1.2.840.113549.1.1", "RSA");
	
	map.put("Alg.Alias.KeyPairGenerator.1.2.840.113549.1.1",     "RSA");
	map.put("Alg.Alias.KeyPairGenerator.OID.1.2.840.113549.1.1", "RSA");

	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.2",     "MD2withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.2", "MD2withRSA");
	
	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.4",     "MD5withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.4", "MD5withRSA");
	
	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.5",     "SHA1withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.5", "SHA1withRSA");
	map.put("Alg.Alias.Signature.1.3.14.3.2.29",            "SHA1withRSA");

	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.11",     "SHA256withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.11", "SHA256withRSA");
	
	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.12",     "SHA384withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.12", "SHA384withRSA");
	
	map.put("Alg.Alias.Signature.1.2.840.113549.1.1.13",     "SHA512withRSA");
	map.put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.13", "SHA512withRSA");
	
	if (map != this) {
	    AccessController.doPrivileged(new PutAllAction(this, map));
	}
	
    }
}

