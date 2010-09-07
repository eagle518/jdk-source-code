/*
 * @(#)MSCryptoPrivateKey.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.deploy.security;

import java.security.PrivateKey;

/**
 * The base class to an RSA or DSA private key for Microsoft Crypto APIs.
 *
 * @see MSCryptoPrivateKey
 *
 * @since 1.5
 * @author  Stanley Man-Kit Ho 
 */
abstract class MSCryptoPrivateKey implements PrivateKey
{

    // Native handle
    protected int hCryptProv = 0;
    protected int hCryptKey = 0;
    
    // Key length
    protected int keyLength = 0;

    // Flag to track if finalize() has been called once
    private boolean finalized = false;

    /**
     * Construct a MSCryptoPrivateKey object.
     */
    protected MSCryptoPrivateKey(int hCryptProv, int hCryptKey, int keyLength)
    {
	this.hCryptProv = hCryptProv;
	this.hCryptKey = hCryptKey;
	this.keyLength = keyLength;
    }

    /**
     * Finalization method
     */
    public void finalize()
    {
	try
	{
	    synchronized(this)
	    {
		if (finalized == false)
		{
		    cleanUp(hCryptProv, hCryptKey);
		    hCryptProv = 0;
		    hCryptKey = 0;
		    finalized = true;

		    super.finalize();
		}
	    }
	} 
	catch (Throwable e)
	{
	    e.printStackTrace();
	}
    }

    /**
     * Native method to cleanup the key handle.
     */
    private native static void cleanUp(int hCryptProv, int hCryptKey);

    /** 
     * Return bit length of the key.
     */
    public int bitLength()
    {
	return keyLength;
    }

    
    /**
     * Return native HCRYPTKEY handle.
     */
    public int getHCryptKey()
    {
	return hCryptKey;
    }    

    /**
     * Return native HCRYPTPROV handle.
     */
    public int getHCryptProvider()
    {
	return hCryptProv;
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
    public abstract String getAlgorithm();

    /**
     * Returns the name of the primary encoding format of this key,
     * or null if this key does not support encoding.
     * The primary encoding format is
     * named in terms of the appropriate ASN.1 data format, if an
     * ASN.1 specification for this key exists.
     * For example, the name of the ASN.1 data format for public
     * keys is <I>SubjectPublicKeyInfo</I>, as
     * defined by the X.509 standard; in this case, the returned format is
     * <code>"X.509"</code>. Similarly,
     * the name of the ASN.1 data format for private keys is
     * <I>PrivateKeyInfo</I>,
     * as defined by the PKCS #8 standard; in this case, the returned format is
     * <code>"PKCS#8"</code>.
     *
     * @return the primary encoding format of the key.
     */
    public String getFormat()
    {
	return null;
    }

    /**
     * Returns the key in its primary encoding format, or null
     * if this key does not support encoding.
     *
     * @return the encoded key, or null if the key does not support
     * encoding.
     */
    public byte[] getEncoded()
    {
	return null;
    } 
}
