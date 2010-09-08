/*
 * @(#)KeyGenerator.java	1.13 10/03/23
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
 * This class provides the functionality of a secret (symmetric) key generator.
 *
 * <p>Key generators are constructed using one of the <code>getInstance</code>
 * class methods of this class.
 *
 * <p>KeyGenerator objects are reusable, i.e., after a key has been
 * generated, the same KeyGenerator object can be re-used to generate further
 * keys.
 *
 * <p>There are two ways to generate a key: in an algorithm-independent
 * manner, and in an algorithm-specific manner.
 * The only difference between the two is the initialization of the object:
 *
 * <ul>
 * <li><b>Algorithm-Independent Initialization</b>
 * <p>All key generators share the concepts of a <i>keysize</i> and a
 * <i>source of randomness</i>.
 * There is an
 * {@link #init(int, java.security.SecureRandom) init}
 * method in this KeyGenerator class that takes these two universally
 * shared types of arguments. There is also one that takes just a
 * <code>keysize</code> argument, and uses the SecureRandom implementation
 * of the highest-priority installed provider as the source of randomness
 * (or a system-provided source of randomness if none of the installed
 * providers supply a SecureRandom implementation), and one that takes just a
 * source of randomness.
 *
 * <p>Since no other parameters are specified when you call the above
 * algorithm-independent <code>init</code> methods, it is up to the
 * provider what to do about the algorithm-specific parameters (if any) to be
 * associated with each of the keys.
 * <p>
 *
 * <li><b>Algorithm-Specific Initialization</b>
 * <p>For situations where a set of algorithm-specific parameters already
 * exists, there are two
 * {@link #init(java.security.spec.AlgorithmParameterSpec) init}
 * methods that have an <code>AlgorithmParameterSpec</code>
 * argument. One also has a <code>SecureRandom</code> argument, while the
 * other uses the SecureRandom implementation
 * of the highest-priority installed provider as the source of randomness
 * (or a system-provided source of randomness if none of the installed
 * providers supply a SecureRandom implementation).
 * </ul>
 *
 * <p>In case the client does not explicitly initialize the KeyGenerator
 * (via a call to an <code>init</code> method), each provider must
 * supply (and document) a default initialization.
 *
 * @author Jan Luehe
 *
 * @version 1.52, 04/24/06
 *
 * @see SecretKey
 * @since 1.4
 */
public class KeyGenerator
{

    /** 
     * Creates a KeyGenerator object.
     *
     * @param keyGenSpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected KeyGenerator(KeyGeneratorSpi keyGenSpi, Provider provider, String
        algorithm)
    { }

    /** 
     * Returns the algorithm name of this <code>KeyGenerator</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>KeyGenerator</code> object.
     *
     * @return the algorithm name of this <code>KeyGenerator</code> object.
     */
    public final String getAlgorithm() {
        return null;
    }

    /** 
     * Returns a <code>KeyGenerator</code> object that generates secret keys
     * for the specified algorithm.
     *
     * <p> This method traverses the list of registered security Providers,
     * starting with the most preferred Provider.
     * A new KeyGenerator object encapsulating the
     * KeyGeneratorSpi implementation from the first
     * Provider that supports the specified algorithm is returned.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested key algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @return the new <code>KeyGenerator</code> object.
     *
     * @exception NullPointerException if the specified algorithm is null.
     *
     * @exception NoSuchAlgorithmException if no Provider supports a
     *		KeyGeneratorSpi implementation for the
     *		specified algorithm.
     *
     * @see java.security.Provider
     */
    public static final KeyGenerator getInstance(String algorithm)
        throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyGenerator</code> object that generates secret keys
     * for the specified algorithm.
     *
     * <p> A new KeyGenerator object encapsulating the
     * KeyGeneratorSpi implementation from the specified provider
     * is returned.  The specified provider must be registered
     * in the security provider list.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested key algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the name of the provider.
     *
     * @return the new <code>KeyGenerator</code> object.
     *
     * @exception NullPointerException if the specified algorithm is null.
     *
     * @exception NoSuchAlgorithmException if a KeyGeneratorSpi
     *		implementation for the specified algorithm is not
     *		available from the specified provider.
     *
     * @exception NoSuchProviderException if the specified provider is not
     *		registered in the security provider list.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null or empty.
     *
     * @see java.security.Provider
     */
    public static final KeyGenerator getInstance(String algorithm, String
        provider) throws NoSuchAlgorithmException, NoSuchProviderException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyGenerator</code> object that generates secret keys
     * for the specified algorithm.
     *
     * <p> A new KeyGenerator object encapsulating the
     * KeyGeneratorSpi implementation from the specified Provider
     * object is returned.  Note that the specified Provider object
     * does not have to be registered in the provider list.
     *
     * @param algorithm the standard name of the requested key algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the provider.
     *
     * @return the new <code>KeyGenerator</code> object.
     *
     * @exception NullPointerException if the specified algorithm is null.
     *
     * @exception NoSuchAlgorithmException if a KeyGeneratorSpi
     *		implementation for the specified algorithm is not available
     *		from the specified Provider object.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null.
     *
     * @see java.security.Provider
     */
    public static final KeyGenerator getInstance(String algorithm, Provider
        provider) throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns the provider of this <code>KeyGenerator</code> object.
     *
     * @return the provider of this <code>KeyGenerator</code> object
     */
    public final Provider getProvider() {
        return null;
    }

    /** 
     * Initializes this key generator.
     *
     * @param random the source of randomness for this generator
     */
    public final void init(SecureRandom random) { }

    /** 
     * Initializes this key generator with the specified parameter set.
     *
     * <p> If this key generator requires any random bytes, it will get them
     * using the
     * {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority installed
     * provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * @param params the key generation parameters
     *
     * @exception InvalidAlgorithmParameterException if the given parameters
     * are inappropriate for this key generator
     */
    public final void init(AlgorithmParameterSpec params)
        throws InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this key generator with the specified parameter
     * set and a user-provided source of randomness.
     *
     * @param params the key generation parameters
     * @param random the source of randomness for this key generator
     *
     * @exception InvalidAlgorithmParameterException if <code>params</code> is
     * inappropriate for this key generator
     */
    public final void init(AlgorithmParameterSpec params, SecureRandom random)
        throws InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this key generator for a certain keysize.
     *
     * <p> If this key generator requires any random bytes, it will get them
     * using the
     * {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority installed
     * provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * @param keysize the keysize. This is an algorithm-specific metric,
     * specified in number of bits.
     *
     * @exception InvalidParameterException if the keysize is wrong or not
     * supported.
     */
    public final void init(int keysize) { }

    /** 
     * Initializes this key generator for a certain keysize, using a
     * user-provided source of randomness.
     *
     * @param keysize the keysize. This is an algorithm-specific metric,
     * specified in number of bits.
     * @param random the source of randomness for this key generator
     *
     * @exception InvalidParameterException if the keysize is wrong or not
     * supported.
     */
    public final void init(int keysize, SecureRandom random) { }

    /** 
     * Generates a secret key.
     *
     * @return the new key
     */
    public final SecretKey generateKey() {
        return null;
    }
}
