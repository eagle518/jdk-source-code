/*
 * @(#)MozillaJSSNONEwithRSASignature.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.Key;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.InvalidKeyException;
import java.security.InvalidParameterException;
import java.security.InvalidAlgorithmParameterException;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.SignatureSpi;
import java.security.SignatureException;
import java.lang.reflect.*;
import sun.security.rsa.RSAPadding;
import sun.security.rsa.RSACore;
import com.sun.deploy.util.Trace;

/**
 * NONEwithRSA signature implementation. Supports RSA signing using PKCS#1 v1.5 padding.
 *
 * Objects should be instantiated by calling Signature.getInstance() using the
 * following algorithm names:
 *  . "NONEwithRSA" for PKCS#1 padding. 
 *
 * Note: RSA keys should be at least 512 bits long
 *
 * @since   1.5
 * @author  Stanley Man-Kit Ho
 */
public final class MozillaJSSNONEwithRSASignature extends java.security.SignatureSpi 
{
    // buffer for the data
    private byte[] buffer;
    // offset into the buffer (number of bytes buffered)
    private int bufOfs;
    
    // the private key, if we were initialized using a private key
    private MozillaJSSRSAPrivateKey privateKey = null;
    
    public MozillaJSSNONEwithRSASignature() 
    {
    }
   
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
	// This signature could only accept MozillaJSSRSAPrivateKey
	if ((key instanceof MozillaJSSRSAPrivateKey) == false)
	    throw new InvalidKeyException("Key not supported");
	    	
	privateKey = (MozillaJSSRSAPrivateKey) key;	
	
	// Determine byte length from bit length	
	int keySize = (privateKey.bitLength() + 7) >> 3;
	
	if (keySize < 64)
	    throw new InvalidKeyException("RSA keys should be at least 512 bits long");

	bufOfs = 0;

	try {
	    RSAPadding padding = RSAPadding.getInstance(RSAPadding.PAD_BLOCKTYPE_1, keySize, appRandom);
	    int maxDataSize = padding.getMaxDataSize();
	    buffer = new byte[maxDataSize];	    
	}catch(InvalidAlgorithmParameterException exc) {
	    Trace.securityPrintException(exc);
	}
    }
	
    // internal update method
    private void update(byte[] in, int inOfs, int inLen) 
    {
	if ((inLen == 0) || (in == null)) 
	    return;

	if (bufOfs + inLen > buffer.length) 
	{
	    bufOfs = buffer.length + 1;
	    return;
	}
	System.arraycopy(in, inOfs, buffer, bufOfs, inLen);
	bufOfs += inLen;
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
	if (bufOfs > buffer.length) 
	{
	    throw new SignatureException("Data must not be longer "
		+ "than " + buffer.length + " bytes");
	}

	try 
	{
	    // Copy hash data from buffer into a separate byte array
	    byte[] hash = RSACore.convert(buffer, 0, bufOfs);
	    Object result = null;
	    
	    try
	    {
		// Obtain JSS private key
		Object pk = privateKey.getJSSPrivateKey();
		//org.mozilla.jss.crypto.PrivateKey pk = (org.mozilla.jss.crypto.PrivateKey) privateKey.getJSSPrivateKey();
		
		// Determine crypto token associated with JSS private key
		Class jssprivkey = Class.forName("org.mozilla.jss.crypto.PrivateKey", true,
						  ClassLoader.getSystemClassLoader());
		Method getOwnTokenMeth = jssprivkey.getMethod("getOwningToken", null);
		Object token = getOwnTokenMeth.invoke(pk, null);
		//org.mozilla.jss.crypto.CryptoToken token = pk.getOwningToken();
		
		// Obtain RSA raw signature associated with crypto token
		Class jsscryptotoken = Class.forName("org.mozilla.jss.crypto.CryptoToken", true,
						      ClassLoader.getSystemClassLoader());
		Class jsssigalgo = Class.forName("org.mozilla.jss.crypto.SignatureAlgorithm", true,
						  ClassLoader.getSystemClassLoader());

		Class partypes[] = new Class[] {jsssigalgo};
		Method getSigConMeth = jsscryptotoken.getMethod("getSignatureContext", partypes);

		Field rsasigfield = jsssigalgo.getField("RSASignature");
		Object arglist[] = new Object[1];
   		arglist[0] = rsasigfield.get(privateKey);
		Object sig = getSigConMeth.invoke(token, arglist);
		//org.mozilla.jss.crypto.Signature sig = token.getSignatureContext(org.mozilla.jss.crypto.SignatureAlgorithm.RSASignature);

		// Sign hash using NONEwithRSA signature through Mozilla JSS 	    
		Class jsssignature = Class.forName("org.mozilla.jss.crypto.Signature", true,
						    ClassLoader.getSystemClassLoader());

		Class partypes2[] = new Class[] {jssprivkey};
		Method initSignMeth = jsssignature.getMethod("initSign", partypes2);

		Object arglist2[] = new Object[] {pk};
		Object siginit = initSignMeth.invoke(sig, arglist2);
		//sig.initSign(pk);

		Class partypes3[] = new Class[] {byte[].class};
		Method updateMeth = jsssignature.getMethod("update", partypes3);
		Object arglist3[] = new Object[] {hash};
		Object sigupdate = updateMeth.invoke(sig, arglist3);
		//sig.update(hash);

		Method signMeth = jsssignature.getMethod("sign", null);
		result = signMeth.invoke(sig, null);
		//result = sig.sign();
	    }
	    catch (Throwable e)
	    {
		e.printStackTrace();
	    }

	    return (byte[]) result;
	} 
	finally 
	{
	    bufOfs = 0;
	}    
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
}


