/*
 * @(#)KeyManagerFactory.java	1.15 10/03/23
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

package javax.net.ssl;

import java.security.*;

import java.security.Security;
import sun.security.jca.GetInstance;

/** 
 * This class acts as a factory for key managers based on a
 * source of key material. Each key manager manages a specific
 * type of key material for use by secure sockets. The key
 * material is based on a KeyStore and/or provider specific sources.
 *
 * @since 1.4
 * @see KeyManager
 * @version 1.21, 04/24/06
 */
public class KeyManagerFactory
{

    /** 
     * Creates a KeyManagerFactory object.
     *
     * @param factorySpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected KeyManagerFactory(KeyManagerFactorySpi factorySpi, Provider
        provider, String algorithm)
    { }

    /** 
     * Obtains the default KeyManagerFactory algorithm name.
     *
     * <p>The default algorithm can be changed at runtime by setting
     * the value of the "ssl.KeyManagerFactory.algorithm" security
     * property (set in the Java security properties file or by calling
     * {@link java.security.Security#setProperty(java.lang.String,
     * java.lang.String)})
     * to the desired algorithm name.
     *
     * @see java.security.Security#setProperty(java.lang.String,
     *		java.lang.String)
     * @return the default algorithm name as specified in the
     *		Java security properties, or an implementation-specific
     *		default if no such property exists.
     */
    public static final String getDefaultAlgorithm() {
        return null;
    }

    /** 
     * Returns the algorithm name of this <code>KeyManagerFactory</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>KeyManagerFactory</code> object.
     *
     * @return the algorithm name of this <code>KeyManagerFactory</code> object.
     */
    public final String getAlgorithm() {
        return null;
    }

    /** 
     * Returns a <code>KeyManagerFactory</code> object that acts as a
     * factory for key managers.
     *
     * <p> This method traverses the list of registered security Providers,
     * starting with the most preferred Provider.
     * A new KeyManagerFactory object encapsulating the
     * KeyManagerFactorySpi implementation from the first
     * Provider that supports the specified algorithm is returned.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested algorithm.
     *		See the <a href=
     *	"{@docRoot}/../technotes/guides/security/jsse/JSSERefGuide.html">
     *		Java Secure Socket Extension Reference Guide </a>
     *		for information about standard algorithm names.
     *
     * @return the new <code>KeyManagerFactory</code> object.
     *
     * @exception NoSuchAlgorithmException if no Provider supports a
     *		KeyManagerFactorySpi implementation for the
     *		specified algorithm.
     *
     * @see java.security.Provider
     */
    public static final KeyManagerFactory getInstance(String algorithm)
        throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyManagerFactory</code> object that acts as a
     * factory for key managers.
     *
     * <p> A new KeyManagerFactory object encapsulating the
     * KeyManagerFactorySpi implementation from the specified provider
     * is returned.  The specified provider must be registered
     * in the security provider list.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested algorithm.
     *		See the <a href=
     *	"{@docRoot}/../technotes/guides/security/jsse/JSSERefGuide.html">
     *		Java Secure Socket Extension Reference Guide </a>
     *		for information about standard algorithm names.
     *
     * @param provider the name of the provider.
     *
     * @return the new <code>KeyManagerFactory</code> object.
     *
     * @throws NoSuchAlgorithmException if a KeyManagerFactorySpi
     *		implementation for the specified algorithm is not
     *		available from the specified provider.
     *
     * @throws NoSuchProviderException if the specified provider is not
     *		registered in the security provider list.
     *
     * @throws IllegalArgumentException if the provider name is null or empty.
     *
     * @see java.security.Provider
     */
    public static final KeyManagerFactory getInstance(String algorithm, String
        provider) throws NoSuchAlgorithmException, NoSuchProviderException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyManagerFactory</code> object that acts as a
     * factory for key managers.
     *
     * <p> A new KeyManagerFactory object encapsulating the
     * KeyManagerFactorySpi implementation from the specified Provider
     * object is returned.  Note that the specified Provider object
     * does not have to be registered in the provider list.
     *
     * @param algorithm the standard name of the requested algorithm.
     *		See the <a href=
     *	"{@docRoot}/../technotes/guides/security/jsse/JSSERefGuide.html">
     *		Java Secure Socket Extension Reference Guide </a>
     *		for information about standard algorithm names.
     *
     * @param provider an instance of the provider.
     *
     * @return the new <code>KeyManagerFactory</code> object.
     *
     * @throws NoSuchAlgorithmException if a KeyManagerFactorySpi
     *		implementation for the specified algorithm is not available
     *		from the specified Provider object.
     *
     * @throws IllegalArgumentException if provider is null.
     *
     * @see java.security.Provider
     */
    public static final KeyManagerFactory getInstance(String algorithm, Provider
        provider) throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns the provider of this <code>KeyManagerFactory</code> object.
     *
     * @return the provider of this <code>KeyManagerFactory</code> object
     */
    public final Provider getProvider() {
        return null;
    }

    /** 
     * Initializes this factory with a source of key material.
     * <P>
     * The provider typically uses a KeyStore for obtaining
     * key material for use during secure socket negotiations.
     * The KeyStore is generally password-protected.
     * <P>
     * For more flexible initialization, please see
     * {@link #init(ManagerFactoryParameters)}.
     * <P>
     *
     * @param ks the key store or null
     * @param password the password for recovering keys in the KeyStore
     * @throws KeyStoreException if this operation fails
     * @throws NoSuchAlgorithmException if the specified algorithm is not
     *		available from the specified provider.
     * @throws UnrecoverableKeyException if the key cannot be recovered
     *		(e.g. the given password is wrong).
     */
    public final void init(KeyStore ks, char[] password)
        throws KeyStoreException, NoSuchAlgorithmException,
        UnrecoverableKeyException
    { }

    /** 
     * Initializes this factory with a source of provider-specific
     * key material.
     * <P>
     * In some cases, initialization parameters other than a keystore
     * and password may be needed by a provider.  Users of that
     * particular provider are expected to pass an implementation of
     * the appropriate <CODE>ManagerFactoryParameters</CODE> as
     * defined by the provider.  The provider can then call the
     * specified methods in the <CODE>ManagerFactoryParameters</CODE>
     * implementation to obtain the needed information.
     *
     * @param spec an implementation of a provider-specific parameter
     *		specification
     * @throws InvalidAlgorithmParameterException if an error is encountered
     */
    public final void init(ManagerFactoryParameters spec)
        throws InvalidAlgorithmParameterException
    { }

    /** 
     * Returns one key manager for each type of key material.
     *
     * @return the key managers
     * @throws IllegalStateException if the KeyManagerFactory is not initialized
     */
    public final KeyManager[] getKeyManagers() {
        return null;
    }
}
