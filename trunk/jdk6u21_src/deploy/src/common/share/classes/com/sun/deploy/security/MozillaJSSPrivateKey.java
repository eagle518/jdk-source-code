/*
 * @(#)MozillaJSSPrivateKey.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.deploy.security;

import java.security.PrivateKey;

/**
 * The base class to an RSA or DSA private key for Mozilla JSS APIs.
 *
 * @see MozillaJSSPrivateKey
 *
 * @since 1.5
 * @author  Stanley Man-Kit Ho 
 */
abstract class MozillaJSSPrivateKey implements PrivateKey
{

    // Native handle
    protected Object key = null;
    
    // Key length
    protected int keyLength = 0;

    /**
     * Construct a MozillaJSSPrivateKey object.
     */
    protected MozillaJSSPrivateKey(Object key, int keyLength)
    {
	this.key = key;
	this.keyLength = keyLength;
    }

    /** 
     * Return bit length of the key.
     */
    public int bitLength()
    {
	return keyLength;
    }

    /**
     * Return JSS private key.
     */
    public Object getJSSPrivateKey()
    {
	return key;
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
