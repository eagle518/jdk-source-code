/*
 * @(#)RSAPublicKey.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.mscapi;

import java.math.BigInteger;
import java.security.InvalidKeyException;
import java.security.KeyException;
import java.security.KeyRep;
import java.security.ProviderException;
import java.security.PublicKey;

import sun.security.rsa.RSAPublicKeyImpl;

/**
 * The handle for an RSA public key using the Microsoft Crypto API.
 *
 * @since 1.6
 */
class RSAPublicKey extends Key implements java.security.interfaces.RSAPublicKey
{
    private byte[] publicKeyBlob = null;
    private byte[] encoding = null;
    private BigInteger modulus = null;
    private BigInteger exponent = null;

    /**
     * Construct an RSAPublicKey object.
     */
    RSAPublicKey(long hCryptProv, long hCryptKey, int keyLength)
    {
	super(hCryptProv, hCryptKey, keyLength);
    }
    
    /**
     * Returns the standard algorithm name for this key. For
     * example, "RSA" would indicate that this key is a RSA key.
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
        
    /**
     * Returns a printable description of the key.
     */
    public String toString()
    {
	StringBuffer sb = new StringBuffer();

	sb.append("RSAPublicKey [size=").append(keyLength)
	    .append(" bits, type=").append(getKeyType(hCryptKey))
	    .append(", container=").append(getContainerName(hCryptProv))
	    .append("]\n  modulus: ").append(getModulus())
	    .append("\n  public exponent: ").append(getPublicExponent());

	return sb.toString();
    }

    /**
     * Returns the public exponent.
     *
     * @exception ProviderException if MS-CAPI cannot extract the public key
     *            from its internal format.
     */
    public BigInteger getPublicExponent() {

	try {
	    if (exponent == null) {
	        publicKeyBlob = getPublicKeyBlob(hCryptKey);
	        exponent = new BigInteger(getExponent(publicKeyBlob));
	    }

	} catch (KeyException ke) {
	    // Wrap the JNI exception in a RuntimeException
	    throw new ProviderException(ke);
	}

	return exponent;
    }

    /**
     * Returns the modulus.
     *
     * @exception ProviderException if MS-CAPI cannot extract the public key
     *            from its internal format.
     */
    public BigInteger getModulus() {

	try {
	    if (modulus == null) {
	        publicKeyBlob = getPublicKeyBlob(hCryptKey);
	        modulus = new BigInteger(getModulus(publicKeyBlob));
	    }

	} catch (KeyException ke) {
	    // Wrap the JNI exception in a RuntimeException
	    throw new ProviderException(ke);
	}

	return modulus;
    }

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
        return "X.509";
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
	if (encoding == null) {

	    try {
		encoding = new RSAPublicKeyImpl(getModulus(), 
		    getPublicExponent()).getEncoded();

	    } catch (InvalidKeyException e) {
		// ignore
	    }
	}
        return encoding;
    }

    protected Object writeReplace() throws java.io.ObjectStreamException {
	return new KeyRep(KeyRep.Type.PUBLIC,
                        getAlgorithm(),
                        getFormat(),
                        getEncoded());
    }

    /*
     * Returns the Microsoft CryptoAPI representation of the key.
     */
    private native byte[] getPublicKeyBlob(long hCryptKey) throws KeyException;

    /*
     * Returns the key's public exponent (in big-endian 2's complement format).
     */
    private native byte[] getExponent(byte[] keyBlob) throws KeyException;

    /*
     * Returns the key's modulus (in big-endian 2's complement format).
     */
    private native byte[] getModulus(byte[] keyBlob) throws KeyException;
}
