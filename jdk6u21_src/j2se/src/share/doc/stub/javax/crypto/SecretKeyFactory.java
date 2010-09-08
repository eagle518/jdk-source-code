/*
 * @(#)SecretKeyFactory.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.util.*;
import java.security.*;
import java.security.spec.*;
import sun.security.jca.*;

import java.security.Provider.Service;
import sun.security.jca.GetInstance.Instance;

/** 
 * This class represents a factory for secret keys.
 *
 * <P> Key factories are used to convert <I>keys</I> (opaque
 * cryptographic keys of type <code>Key</code>) into <I>key specifications</I>
 * (transparent representations of the underlying key material), and vice
 * versa.
 * Secret key factories operate only on secret (symmetric) keys.
 *
 * <P> Key factories are bi-directional, i.e., they allow to build an opaque
 * key object from a given key specification (key material), or to retrieve
 * the underlying key material of a key object in a suitable format.
 *
 * <P> Application developers should refer to their provider's documentation
 * to find out which key specifications are supported by the
 * {@link #generateSecret(java.security.spec.KeySpec) generateSecret} and
 * {@link #getKeySpec(javax.crypto.SecretKey, java.lang.Class) getKeySpec}
 * methods.
 * For example, the DES secret-key factory supplied by the "SunJCE" provider
 * supports <code>DESKeySpec</code> as a transparent representation of DES
 * keys, and that provider's secret-key factory for Triple DES keys supports
 * <code>DESedeKeySpec</code> as a transparent representation of Triple DES
 * keys.
 *
 * @author Jan Luehe
 *
 * @version 1.41, 04/24/06
 *
 * @see SecretKey
 * @see javax.crypto.spec.DESKeySpec
 * @see javax.crypto.spec.DESedeKeySpec
 * @see javax.crypto.spec.PBEKeySpec
 * @since 1.4
 */
public class SecretKeyFactory
{

    /** 
     * Creates a SecretKeyFactory object.
     *
     * @param keyFacSpi the delegate
     * @param provider the provider
     * @param algorithm the secret-key algorithm
     */
    protected SecretKeyFactory(SecretKeyFactorySpi keyFacSpi, Provider provider,
        String algorithm)
    { }

    /** 
     * Returns a <code>SecretKeyFactory</code> object that converts
     * secret keys of the specified algorithm.
     *
     * <p> This method traverses the list of registered security Providers,
     * starting with the most preferred Provider.
     * A new SecretKeyFactory object encapsulating the
     * SecretKeyFactorySpi implementation from the first
     * Provider that supports the specified algorithm is returned.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested secret-key
     * algorithm.
     * See Appendix A in the <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @return the new <code>SecretKeyFactory</code> object.
     *
     * @exception NullPointerException if the specified algorithm
     *		is null.
     *
     * @exception NoSuchAlgorithmException if no Provider supports a
     *		SecretKeyFactorySpi implementation for the
     *		specified algorithm.
     *
     * @see java.security.Provider
     */
    public static final SecretKeyFactory getInstance(String algorithm)
        throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns a <code>SecretKeyFactory</code> object that converts
     * secret keys of the specified algorithm.
     *
     * <p> A new SecretKeyFactory object encapsulating the
     * SecretKeyFactorySpi implementation from the specified provider
     * is returned.  The specified provider must be registered
     * in the security provider list.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested secret-key
     * algorithm.
     * See Appendix A in the <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the name of the provider.
     *
     * @return the new <code>SecretKeyFactory</code> object.
     *
     * @exception NoSuchAlgorithmException if a SecretKeyFactorySpi
     *		implementation for the specified algorithm is not
     *		available from the specified provider.
     *
     * @exception NullPointerException if the specified algorithm
     *		is null.
     *
     * @throws NoSuchProviderException if the specified provider is not
     *		registered in the security provider list.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null or empty.
     *
     * @see java.security.Provider
     */
    public static final SecretKeyFactory getInstance(String algorithm, String
        provider) throws NoSuchAlgorithmException, NoSuchProviderException
    {
        return null;
    }

    /** 
     * Returns a <code>SecretKeyFactory</code> object that converts
     * secret keys of the specified algorithm.
     *
     * <p> A new SecretKeyFactory object encapsulating the
     * SecretKeyFactorySpi implementation from the specified Provider
     * object is returned.  Note that the specified Provider object
     * does not have to be registered in the provider list.
     *
     * @param algorithm the standard name of the requested secret-key
     * algorithm.
     * See Appendix A in the <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the provider.
     *
     * @return the new <code>SecretKeyFactory</code> object.
     *
     * @exception NullPointerException if the specified algorithm
     * is null.
     *
     * @exception NoSuchAlgorithmException if a SecretKeyFactorySpi
     *		implementation for the specified algorithm is not available
     *		from the specified Provider object.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null.
     *
     * @see java.security.Provider
     */
    public static final SecretKeyFactory getInstance(String algorithm, Provider
        provider) throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns the provider of this <code>SecretKeyFactory</code> object.
     *
     * @return the provider of this <code>SecretKeyFactory</code> object
     */
    public final Provider getProvider() {
        return null;
    }

    /** 
     * Returns the algorithm name of this <code>SecretKeyFactory</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>SecretKeyFactory</code> object.
     *
     * @return the algorithm name of this <code>SecretKeyFactory</code>
     * object.
     */
    public final String getAlgorithm() {
        return null;
    }

    /** 
     * Generates a <code>SecretKey</code> object from the provided key
     * specification (key material).
     *
     * @param keySpec the specification (key material) of the secret key
     *
     * @return the secret key
     *
     * @exception InvalidKeySpecException if the given key specification
     * is inappropriate for this secret-key factory to produce a secret key.
     */
    public final SecretKey generateSecret(KeySpec keySpec)
        throws InvalidKeySpecException
    {
        return null;
    }

    /** 
     * Returns a specification (key material) of the given key object
     * in the requested format.
     *
     * @param key the key
     * @param keySpec the requested format in which the key material shall be
     * returned
     *
     * @return the underlying key specification (key material) in the
     * requested format
     *
     * @exception InvalidKeySpecException if the requested key specification is
     * inappropriate for the given key (e.g., the algorithms associated with
     * <code>key</code> and <code>keySpec</code> do not match, or
     * <code>key</code> references a key on a cryptographic hardware device
     * whereas <code>keySpec</code> is the specification of a software-based
     * key), or the given key cannot be dealt with
     * (e.g., the given key has an algorithm or format not supported by this
     * secret-key factory).
     */
    public final KeySpec getKeySpec(SecretKey key, Class keySpec)
        throws InvalidKeySpecException
    {
        return null;
    }

    /** 
     * Translates a key object, whose provider may be unknown or potentially
     * untrusted, into a corresponding key object of this secret-key factory.
     *
     * @param key the key whose provider is unknown or untrusted
     *
     * @return the translated key
     *
     * @exception InvalidKeyException if the given key cannot be processed
     * by this secret-key factory.
     */
    public final SecretKey translateKey(SecretKey key)
        throws InvalidKeyException
    {
        return null;
    }
}
