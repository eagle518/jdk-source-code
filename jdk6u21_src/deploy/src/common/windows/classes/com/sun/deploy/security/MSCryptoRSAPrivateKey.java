/*
 * @(#)MSCryptoRSAPrivateKey.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.Key;
import java.security.PrivateKey;

/**
 * This class encapsulates the handle of the RSA private key in Microsoft Crypto APIs.
 *
 * @author Stanley Man-Kit Ho
 */
class MSCryptoRSAPrivateKey extends MSCryptoPrivateKey 
{
    /**
     * Construct a MSCryptoRSAPrivateKey object.
     */
    MSCryptoRSAPrivateKey(int hCryptProv, int hCryptKey, int keyLength)
    {
	super(hCryptProv, hCryptKey, keyLength);
    }
    
    /**
     * Returns the standard algorithm name for this key. For
     * example, "DSA" would indicate that this key is a DSA key.
     * See Appendix A in the <a href=
     * "../../../guide/security/CryptoSpec.html#AppA">
     * Java Cryptography Architecture API Specification &amp; Reference </a>
     * for information about standard algorithm names.
     *
     * @return the name of the algorithm associated with this key.
     */
    public String getAlgorithm()
    {
	return "RSA";
    }
        
    public String toString()
    {
	return "MSCryptoRSAPrivateKey [HCRYPTPROV=" + hCryptProv + ", HCRYPTKEY=" + hCryptKey + ", key length=" + keyLength + "bits]";
    }
}
