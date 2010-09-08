/*
 * @(#)KeyAgreement.java	1.15 10/03/23
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
import sun.security.util.Debug;
import sun.security.jca.GetInstance.Instance;

/** 
 * This class provides the functionality of a key agreement (or key
 * exchange) protocol.
 * <p>
 * The keys involved in establishing a shared secret are created by one of the
 * key generators (<code>KeyPairGenerator</code> or
 * <code>KeyGenerator</code>), a <code>KeyFactory</code>, or as a result from
 * an intermediate phase of the key agreement protocol.
 *
 * <p> For each of the correspondents in the key exchange, <code>doPhase</code>
 * needs to be called. For example, if this key exchange is with one other
 * party, <code>doPhase</code> needs to be called once, with the
 * <code>lastPhase</code> flag set to <code>true</code>.
 * If this key exchange is
 * with two other parties, <code>doPhase</code> needs to be called twice,
 * the first time setting the <code>lastPhase</code> flag to
 * <code>false</code>, and the second time setting it to <code>true</code>.
 * There may be any number of parties involved in a key exchange.
 *
 * @author Jan Luehe
 *
 * @version 1.49, 06/01/06
 *
 * @see KeyGenerator
 * @see SecretKey
 * @since 1.4
 */
public class KeyAgreement
{

    /** 
     * Creates a KeyAgreement object.
     *
     * @param keyAgreeSpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected KeyAgreement(KeyAgreementSpi keyAgreeSpi, Provider provider,
        String algorithm)
    { }

    /** 
     * Returns the algorithm name of this <code>KeyAgreement</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>KeyAgreement</code> object.
     *
     * @return the algorithm name of this <code>KeyAgreement</code> object.
     */
    public final String getAlgorithm() {
        return null;
    }

    /** 
     * Returns a <code>KeyAgreement</code> object that implements the
     * specified key agreement algorithm.
     *
     * <p> This method traverses the list of registered security Providers,
     * starting with the most preferred Provider.
     * A new KeyAgreement object encapsulating the
     * KeyAgreementSpi implementation from the first
     * Provider that supports the specified algorithm is returned.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested key agreement
     * algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @return the new <code>KeyAgreement</code> object.
     *
     * @exception NullPointerException if the specified algorithm
     *		is null.
     *
     * @exception NoSuchAlgorithmException if no Provider supports a
     *		KeyAgreementSpi implementation for the
     *		specified algorithm.
     *
     * @see java.security.Provider
     */
    public static final KeyAgreement getInstance(String algorithm)
        throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyAgreement</code> object that implements the
     * specified key agreement algorithm.
     *
     * <p> A new KeyAgreement object encapsulating the
     * KeyAgreementSpi implementation from the specified provider
     * is returned.  The specified provider must be registered
     * in the security provider list.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested key agreement
     * algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the name of the provider.
     *
     * @return the new <code>KeyAgreement</code> object.
     *
     * @exception NullPointerException if the specified algorithm
     *		is null.
     *
     * @exception NoSuchAlgorithmException if a KeyAgreementSpi
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
    public static final KeyAgreement getInstance(String algorithm, String
        provider) throws NoSuchAlgorithmException, NoSuchProviderException
    {
        return null;
    }

    /** 
     * Returns a <code>KeyAgreement</code> object that implements the
     * specified key agreement algorithm.
     *
     * <p> A new KeyAgreement object encapsulating the
     * KeyAgreementSpi implementation from the specified Provider
     * object is returned.  Note that the specified Provider object
     * does not have to be registered in the provider list.
     *
     * @param algorithm the standard name of the requested key agreement
     * algorithm.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard algorithm names.
     *
     * @param provider the provider.
     *
     * @return the new <code>KeyAgreement</code> object.
     *
     * @exception NullPointerException if the specified algorithm
     *		is null.
     *
     * @exception NoSuchAlgorithmException if a KeyAgreementSpi
     *		implementation for the specified algorithm is not available
     *		from the specified Provider object.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null.
     *
     * @see java.security.Provider
     */
    public static final KeyAgreement getInstance(String algorithm, Provider
        provider) throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns the provider of this <code>KeyAgreement</code> object.
     *
     * @return the provider of this <code>KeyAgreement</code> object
     */
    public final Provider getProvider() {
        return null;
    }

    /** 
     * Initializes this key agreement with the given key, which is required to
     * contain all the algorithm parameters required for this key agreement.
     *
     * <p> If this key agreement requires any random bytes, it will get
     * them using the
     * {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * @param key the party's private information. For example, in the case
     * of the Diffie-Hellman key agreement, this would be the party's own
     * Diffie-Hellman private key.
     *
     * @exception InvalidKeyException if the given key is
     * inappropriate for this key agreement, e.g., is of the wrong type or
     * has an incompatible algorithm type.
     */
    public final void init(Key key) throws InvalidKeyException { }

    /** 
     * Initializes this key agreement with the given key and source of
     * randomness. The given key is required to contain all the algorithm
     * parameters required for this key agreement.
     *
     * <p> If the key agreement algorithm requires random bytes, it gets them
     * from the given source of randomness, <code>random</code>.
     * However, if the underlying
     * algorithm implementation does not require any random bytes,
     * <code>random</code> is ignored.
     *
     * @param key the party's private information. For example, in the case
     * of the Diffie-Hellman key agreement, this would be the party's own
     * Diffie-Hellman private key.
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is
     * inappropriate for this key agreement, e.g., is of the wrong type or
     * has an incompatible algorithm type.
     */
    public final void init(Key key, SecureRandom random)
        throws InvalidKeyException
    { }

    /** 
     * Initializes this key agreement with the given key and set of
     * algorithm parameters.
     *
     * <p> If this key agreement requires any random bytes, it will get
     * them using the
     * {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * @param key the party's private information. For example, in the case
     * of the Diffie-Hellman key agreement, this would be the party's own
     * Diffie-Hellman private key.
     * @param params the key agreement parameters
     *
     * @exception InvalidKeyException if the given key is
     * inappropriate for this key agreement, e.g., is of the wrong type or
     * has an incompatible algorithm type.
     * @exception InvalidAlgorithmParameterException if the given parameters
     * are inappropriate for this key agreement.
     */
    public final void init(Key key, AlgorithmParameterSpec params)
        throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this key agreement with the given key, set of
     * algorithm parameters, and source of randomness.
     *
     * @param key the party's private information. For example, in the case
     * of the Diffie-Hellman key agreement, this would be the party's own
     * Diffie-Hellman private key.
     * @param params the key agreement parameters
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is
     * inappropriate for this key agreement, e.g., is of the wrong type or
     * has an incompatible algorithm type.
     * @exception InvalidAlgorithmParameterException if the given parameters
     * are inappropriate for this key agreement.
     */
    public final void init(Key key, AlgorithmParameterSpec params, SecureRandom
        random) throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Executes the next phase of this key agreement with the given
     * key that was received from one of the other parties involved in this key
     * agreement.
     *
     * @param key the key for this phase. For example, in the case of
     * Diffie-Hellman between 2 parties, this would be the other party's
     * Diffie-Hellman public key.
     * @param lastPhase flag which indicates whether or not this is the last
     * phase of this key agreement.
     *
     * @return the (intermediate) key resulting from this phase, or null
     * if this phase does not yield a key
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this phase.
     * @exception IllegalStateException if this key agreement has not been
     * initialized.
     */
    public final Key doPhase(Key key, boolean lastPhase)
        throws InvalidKeyException, IllegalStateException
    {
        return null;
    }

    /** 
     * Generates the shared secret and returns it in a new buffer.
     *
     * <p>This method resets this <code>KeyAgreement</code> object, so that it
     * can be reused for further key agreements. Unless this key agreement is
     * reinitialized with one of the <code>init</code> methods, the same
     * private information and algorithm parameters will be used for
     * subsequent key agreements.
     *
     * @return the new buffer with the shared secret
     *
     * @exception IllegalStateException if this key agreement has not been
     * completed yet
     */
    public final byte[] generateSecret() throws IllegalStateException {
        return null;
    }

    /** 
     * Generates the shared secret, and places it into the buffer
     * <code>sharedSecret</code>, beginning at <code>offset</code> inclusive.
     *
     * <p>If the <code>sharedSecret</code> buffer is too small to hold the
     * result, a <code>ShortBufferException</code> is thrown.
     * In this case, this call should be repeated with a larger output buffer.
     *
     * <p>This method resets this <code>KeyAgreement</code> object, so that it
     * can be reused for further key agreements. Unless this key agreement is
     * reinitialized with one of the <code>init</code> methods, the same
     * private information and algorithm parameters will be used for
     * subsequent key agreements.
     *
     * @param sharedSecret the buffer for the shared secret
     * @param offset the offset in <code>sharedSecret</code> where the
     * shared secret will be stored
     *
     * @return the number of bytes placed into <code>sharedSecret</code>
     *
     * @exception IllegalStateException if this key agreement has not been
     * completed yet
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the secret
     */
    public final int generateSecret(byte[] sharedSecret, int offset)
        throws IllegalStateException, ShortBufferException
    {
        return 0;
    }

    /** 
     * Creates the shared secret and returns it as a <code>SecretKey</code>
     * object of the specified algorithm.
     *
     * <p>This method resets this <code>KeyAgreement</code> object, so that it
     * can be reused for further key agreements. Unless this key agreement is
     * reinitialized with one of the <code>init</code> methods, the same
     * private information and algorithm parameters will be used for
     * subsequent key agreements.
     *
     * @param algorithm the requested secret-key algorithm
     *
     * @return the shared secret key
     *
     * @exception IllegalStateException if this key agreement has not been
     * completed yet
     * @exception NoSuchAlgorithmException if the specified secret-key
     * algorithm is not available
     * @exception InvalidKeyException if the shared secret-key material cannot
     * be used to generate a secret key of the specified algorithm (e.g.,
     * the key material is too short)
     */
    public final SecretKey generateSecret(String algorithm)
        throws IllegalStateException, NoSuchAlgorithmException,
        InvalidKeyException
    {
        return null;
    }
}
