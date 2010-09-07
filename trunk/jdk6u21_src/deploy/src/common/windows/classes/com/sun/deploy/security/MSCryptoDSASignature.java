/*
 * @(#)MSCryptoDSASignature.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.math.BigInteger;
import java.security.Key;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.InvalidKeyException;
import java.security.InvalidParameterException;
import java.security.InvalidAlgorithmParameterException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.SignatureSpi;
import java.security.SignatureException;
import com.sun.deploy.util.Trace;
import sun.security.util.DerOutputStream;
import sun.security.util.DerValue;


/**
 * NONEwithDSA and SHA1withDSA signature implementation. Supports DSA signing.
 *
 * Objects should be instantiated by calling Signature.getInstance() using the
 * following algorithm names:
 *  . "NONEwithDSA". 
 *  . "SHA1withDSA". 
 *
 * @since   1.5
 * @author  Stanley Man-Kit Ho
 */
public abstract class MSCryptoDSASignature extends java.security.SignatureSpi 
{
    // the private key, if we were initialized using a private key
    private MSCryptoDSAPrivateKey privateKey = null;
    
    public MSCryptoDSASignature() 
    {
    }

    /**
     * Update an array of byte to be signed or verified.
     */
    protected abstract void update(byte[] data, int off, int len);
	
    /**
     * Return the 20 byte hash value and reset the digest.
     */
    protected abstract byte[] getDigest() throws SignatureException;
       
    /**
     * Reset digest.
     */
    protected abstract void resetDigest();
            
    /**
     * Initializes this signature object with the specified
     * public key for verification operations.
     *
     * @param publicKey the public key of the identity whose signature is
     * going to be verified.
     * 
     * @exception InvalidKeyException if the key is improperly
     * encoded, parameters are missing, and so on.  
     */
    protected void engineInitVerify(PublicKey key)
	throws InvalidKeyException
    {
	throw new InvalidKeyException("Key not supported");
    }
    
    /**
     * Initializes this signature object with the specified
     * private key for signing operations.
     *
     * @param privateKey the private key of the identity whose signature
     * will be generated.
     *
     * @exception InvalidKeyException if the key is improperly
     * encoded, parameters are missing, and so on. 
     */
    protected void engineInitSign(PrivateKey key)
	throws InvalidKeyException
    {
	// This signature could only accept MSCryptoDSAPrivateKey
	if ((key instanceof MSCryptoDSAPrivateKey) == false)
	    throw new InvalidKeyException("Key not supported");
	    	
	privateKey = (MSCryptoDSAPrivateKey) key;	
	
	// Determine byte length from bit length	
	int keySize = (privateKey.bitLength() + 7) >> 3;
	
	if (keySize < 64)
	    throw new InvalidKeyException("DSA keys should be at least 512 bits long");
    }

    /**
     * Updates the data to be signed or verified
     * using the specified byte.
     *
     * @param b the byte to use for the update.
     *
     * @exception SignatureException if the engine is not initialized
     * properly.
     */
    protected void engineUpdate(byte b) throws SignatureException
    {
	byte[] buf = new byte[1];
	buf[0] = b;
	
	update(buf, 0, 1); 
    }

    /**
     * Updates the data to be signed or verified, using the 
     * specified array of bytes, starting at the specified offset.
     *
     * @param b the array of bytes  
     * @param off the offset to start from in the array of bytes 
     * @param len the number of bytes to use, starting at offset
     *
     * @exception SignatureException if the engine is not initialized 
     * properly
     */
    protected void engineUpdate(byte[] b, int off, int len) 
	throws SignatureException    
    {
	update(b, off, len);
    }

    /** 
     * Returns the signature bytes of all the data
     * updated so far.    
     * The format of the signature depends on the underlying 
     * signature scheme.
     *
     * @return the signature bytes of the signing operation's result.
     *
     * @exception SignatureException if the engine is not
     * initialized properly or if this signature algorithm is unable to
     * process the input data provided.
     */
    protected byte[] engineSign() throws SignatureException
    {
	try 
	{
	    // Obtain hash values
	    byte[] hash = getDigest();
	
	    // Sign hash using MS Crypto APIs	    
	    byte[] dssSignature = nativeSignHash(hash, hash.length, privateKey.getHCryptProvider(), privateKey.getHCryptKey());

	    // DSS signature is generated through CryptSignHash(). 
	    // It returns r and s as two 20-byte, little-endian integers.
	    //
	    // However, Java expects the DSS signature to be in DER 
	    // encoding which r and s are big-endian integers.
	    // 
	    // Thus, we need to convert the DSS signature returned from
	    // CryptSignHash into DER-encoded format.
	    //
	    // The maximum DER-encoded signature length is 46 bytes.
	    //

	    // Extract little-endian r and s.
	    byte[] rByteArray = new byte[20];
	    byte[] sByteArray = new byte[20];
	    System.arraycopy(dssSignature, 0, rByteArray, 0, 20);
	    System.arraycopy(dssSignature, 20, sByteArray, 0, 20);
	
	    // Convert r and s into big-endian integers
	    BigInteger r = new BigInteger(convertEndianArray(rByteArray));
	    BigInteger s = new BigInteger(convertEndianArray(sByteArray));

	    // Convert the DSS signature into DER-encoded signature
	    DerOutputStream outseq = new DerOutputStream(100);
	    outseq.putInteger(r);
	    outseq.putInteger(s);
	    DerValue result = new DerValue(DerValue.tag_Sequence, outseq.toByteArray());
	
	    return result.toByteArray();
	}   
	catch(SignatureException e)
	{
	    throw e;
	}
	catch (Throwable e)
	{
	    SignatureException se = new SignatureException("Error generating signature.");
	    se.initCause(e);
	    throw se;
	}	
	finally
	{
	    resetDigest();
	}
    }

    /**
     * Sign hash through RawDSA using Microsoft Crypto API with HCRYPTKEY. The
     * returned byte array is a DSS signature specified to CAPI.
     */
    protected static native byte[] nativeSignHash(byte[] hash, int hashSize, int hCryptProv, int hCryptKey) throws SignatureException; 

    /**
     * Convert array from big endian to little endian, or vice versa.
     */   
    private byte[] convertEndianArray(byte[] byteArray) 
    {
	if (byteArray == null || byteArray.length == 0)
	    return byteArray;
	    
	byte [] retval = new byte[byteArray.length];
	
	// make it big endian
	for (int i=0;i < byteArray.length;i++) 
	    retval[i] = byteArray[byteArray.length - i - 1]; 
	
	return(retval);
    }
    
    /** 
     * Verifies the passed-in signature.   
     * 
     * @param sigBytes the signature bytes to be verified.
     *
     * @return true if the signature was verified, false if not. 
     *
     * @exception SignatureException if the engine is not 
     * initialized properly, the passed-in signature is improperly 
     * encoded or of the wrong type, if this signature algorithm is unable to
     * process the input data provided, etc.
     */
    protected boolean engineVerify(byte[] sigBytes) 
	throws SignatureException
    {
	throw new SignatureException("Signature verification not supported");
    }    
    	
    /**
     * Sets the specified algorithm parameter to the specified
     * value. This method supplies a general-purpose mechanism through
     * which it is possible to set the various parameters of this object. 
     * A parameter may be any settable parameter for the algorithm, such as 
     * a parameter size, or a source of random bits for signature generation 
     * (if appropriate), or an indication of whether or not to perform
     * a specific but optional computation. A uniform algorithm-specific 
     * naming scheme for each parameter is desirable but left unspecified 
     * at this time.
     *
     * @param param the string identifier of the parameter.
     *
     * @param value the parameter value.
     *
     * @exception InvalidParameterException if <code>param</code> is an
     * invalid parameter for this signature algorithm engine,
     * the parameter is already set
     * and cannot be set again, a security exception occurs, and so on. 
     *
     * @deprecated Replaced by {@link 
     * #engineSetParameter(java.security.spec.AlgorithmParameterSpec)
     * engineSetParameter}.
     */
    protected void engineSetParameter(String param, Object value) 
	throws InvalidParameterException
    {
	throw new InvalidParameterException("Parameter not supported");	
    }
    

    /**
     * Gets the value of the specified algorithm parameter. 
     * This method supplies a general-purpose mechanism through which it 
     * is possible to get the various parameters of this object. A parameter
     * may be any settable parameter for the algorithm, such as a parameter 
     * size, or  a source of random bits for signature generation (if 
     * appropriate), or an indication of whether or not to perform a 
     * specific but optional computation. A uniform algorithm-specific 
     * naming scheme for each parameter is desirable but left unspecified 
     * at this time.
     *
     * @param param the string name of the parameter.
     *
     * @return the object that represents the parameter value, or null if
     * there is none.
     *
     * @exception InvalidParameterException if <code>param</code> is an 
     * invalid parameter for this engine, or another exception occurs while
     * trying to get this parameter.
     *
     * @deprecated
     */
    protected Object engineGetParameter(String param)
	throws InvalidParameterException
    {
	throw new InvalidParameterException("Parameter not supported");
    }
    
    /**
     * Standard SHA1withDSA extends MSCryptoDSASignature
     */
    public static class SHA1withDSA extends MSCryptoDSASignature  
    {
	// The SHA has for the data
	private final MessageDigest dataSHA;
	
	public SHA1withDSA() throws NoSuchAlgorithmException  
	{
	    dataSHA = MessageDigest.getInstance("SHA-1");
	    dataSHA.reset();
	}
	
	/**
	 * Update an array of byte to be signed or verified.
	 */
	protected void update(byte[] data, int off, int len) 
	{
	    if (len == 0 || data == null) 
		return;
		
	    dataSHA.update(data, off, len);
	}
	
	/**
	 * Return the 20 byte hash value and reset the digest.
	 */
	protected byte[] getDigest()
	{
	    return dataSHA.digest();
	}
	
	/**
	 * Reset digest.
	 */
	protected void resetDigest()
	{
	    dataSHA.reset();
	} 
    }
        
    /**
     * RawDSA implementation.
     *
     * RawDSA requires the data to be exactly 20 bytes long. If it is
     * not, a SignatureException is thrown when sign()/verify() is
     * called per JCA spec.
     */
    public static final class NONEwithDSA extends MSCryptoDSASignature  
    {
	// length of the SHA-1 digest (20 bytes)
	private static final int SHA1_LEN = 20;
	
	// 20 byte digest buffer
	private final byte[] digestBuffer = new byte[SHA1_LEN];
	
	// offset into the buffer
	private int offset;
	
	public NONEwithDSA()
	{
	}
	
	/**
	 * Update an array of byte to be signed or verified.
	 */
	protected void update(byte[] data, int off, int len) 
	{
	    if (len == 0 || data == null) 
		return;
	
	    if (offset + len > SHA1_LEN)
	    {
		offset = SHA1_LEN + 1;
		return;
	    }	
		
	    System.arraycopy(data, off, digestBuffer, offset, len);
	    offset += len;
	}
	
	/**
	 * Return the 20 byte hash value and reset the digest.
	 */
	protected byte[] getDigest() throws SignatureException
	{
	    if (offset != SHA1_LEN)
		throw new SignatureException("Data for RawDSA must be exactly 20 bytes long");
		
	    offset = 0;
	    return digestBuffer;
	}

	/**
	 * Reset digest.
	 */
	protected void resetDigest()
	{
	    offset = 0;
	} 
    }    
}


