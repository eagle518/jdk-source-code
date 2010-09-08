/*
 * @(#)ExemptionMechanism.java	1.14 10/03/23
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

import java.security.AlgorithmParameters;
import java.security.Provider;
import java.security.Key;
import java.security.Security;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.InvalidKeyException;
import java.security.InvalidAlgorithmParameterException;
import java.security.spec.AlgorithmParameterSpec;
import sun.security.jca.GetInstance.Instance;

/** 
 * This class provides the functionality of an exemption mechanism, examples
 * of which are <i>key recovery</i>, <i>key weakening</i>, and
 * <i>key escrow</i>.
 *
 * <p>Applications or applets that use an exemption mechanism may be granted
 * stronger encryption capabilities than those which don't.
 *
 * @version 1.15, 04/24/06
 * @since 1.4
 */
public class ExemptionMechanism
{

    /** 
     * Creates a ExemptionMechanism object.
     *
     * @param exmechSpi the delegate
     * @param provider the provider
     * @param mechanism the exemption mechanism
     */
    protected ExemptionMechanism(ExemptionMechanismSpi exmechSpi, Provider
        provider, String mechanism)
    { }

    /** 
     * Returns the exemption mechanism name of this
     * <code>ExemptionMechanism</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>ExemptionMechanism</code> object.
     *
     * @return the exemption mechanism name of this
     * <code>ExemptionMechanism</code> object.
     */
    public final String getName() {
        return null;
    }

    /** 
     * Returns an <code>ExemptionMechanism</code> object that implements the
     * specified exemption mechanism algorithm.
     *
     * <p> This method traverses the list of registered security Providers,
     * starting with the most preferred Provider.
     * A new ExemptionMechanism object encapsulating the
     * ExemptionMechanismSpi implementation from the first
     * Provider that supports the specified algorithm is returned.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested exemption
     * mechanism.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard exemption mechanism names.
     *
     * @return the new <code>ExemptionMechanism</code> object.
     *
     * @exception NullPointerException if <code>algorithm</code>
     *		is null.
     *
     * @exception NoSuchAlgorithmException if no Provider supports an
     *		ExemptionMechanismSpi implementation for the
     *		specified algorithm.
     *
     * @see java.security.Provider
     */
    public static final ExemptionMechanism getInstance(String algorithm)
        throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns an <code>ExemptionMechanism</code> object that implements the
     * specified exemption mechanism algorithm.
     *
     * <p> A new ExemptionMechanism object encapsulating the
     * ExemptionMechanismSpi implementation from the specified provider
     * is returned.  The specified provider must be registered
     * in the security provider list.
     *
     * <p> Note that the list of registered providers may be retrieved via
     * the {@link Security#getProviders() Security.getProviders()} method.
     *
     * @param algorithm the standard name of the requested exemption mechanism.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard exemption mechanism names.
     *
     * @param provider the name of the provider.
     *
     * @return the new <code>ExemptionMechanism</code> object.
     *
     * @exception NullPointerException if <code>algorithm</code>
     *		is null.
     *
     * @exception NoSuchAlgorithmException if an ExemptionMechanismSpi
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
    public static final ExemptionMechanism getInstance(String algorithm, String
        provider) throws NoSuchAlgorithmException, NoSuchProviderException
    {
        return null;
    }

    /** 
     * Returns an <code>ExemptionMechanism</code> object that implements the
     * specified exemption mechanism algorithm.
     *
     * <p> A new ExemptionMechanism object encapsulating the
     * ExemptionMechanismSpi implementation from the specified Provider
     * object is returned.  Note that the specified Provider object
     * does not have to be registered in the provider list.
     *
     * @param algorithm the standard name of the requested exemption mechanism.
     * See Appendix A in the
     * <a href=
     *   "{@docRoot}/../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture Reference Guide</a>
     * for information about standard exemption mechanism names.
     *
     * @param provider the provider.
     *
     * @return the new <code>ExemptionMechanism</code> object.
     *
     * @exception NullPointerException if <code>algorithm</code>
     *		is null.
     *
     * @exception NoSuchAlgorithmException if an ExemptionMechanismSpi
     *		implementation for the specified algorithm is not available
     *		from the specified Provider object.
     *
     * @exception IllegalArgumentException if the <code>provider</code>
     *		is null.
     *
     * @see java.security.Provider
     */
    public static final ExemptionMechanism getInstance(String algorithm,
        Provider provider) throws NoSuchAlgorithmException
    {
        return null;
    }

    /** 
     * Returns the provider of this <code>ExemptionMechanism</code> object.
     *
     * @return the provider of this <code>ExemptionMechanism</code> object.
     */
    public final Provider getProvider() {
        return null;
    }

    /** 
     * Returns whether the result blob has been generated successfully by this
     * exemption mechanism.
     *
     * <p>The method also makes sure that the key passed in is the same as
     * the one this exemption mechanism used in initializing and generating
     * phases.
     *
     * @param key the key the crypto is going to use.
     *
     * @return whether the result blob of the same key has been generated
     * successfully by this exemption mechanism; false if <code>key</code>
     * is null.
     *
     * @exception ExemptionMechanismException if problem(s) encountered
     * while determining whether the result blob has been generated successfully
     * by this exemption mechanism object.
     */
    public final boolean isCryptoAllowed(Key key)
        throws ExemptionMechanismException
    {
        return false;
    }

    /** 
     * Returns the length in bytes that an output buffer would need to be in
     * order to hold the result of the next
     * {@link #genExemptionBlob(byte[]) genExemptionBlob}
     * operation, given the input length <code>inputLen</code> (in bytes).
     *
     * <p>The actual output length of the next
     * {@link #genExemptionBlob(byte[]) genExemptionBlob}
     * call may be smaller than the length returned by this method.
     *
     * @param inputLen the input length (in bytes)
     *
     * @return the required output buffer size (in bytes)
     *
     * @exception IllegalStateException if this exemption mechanism is in a
     * wrong state (e.g., has not yet been initialized)
     */
    public final int getOutputSize(int inputLen) throws IllegalStateException {
        return 0;
    }

    /** 
     * Initializes this exemption mechanism with a key.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * that cannot be derived from the given <code>key</code>, the
     * underlying exemption mechanism implementation is supposed to
     * generate the required parameters itself (using provider-specific
     * default values); in the case that algorithm parameters must be
     * specified by the caller, an <code>InvalidKeyException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
    public final void init(Key key)
        throws InvalidKeyException, ExemptionMechanismException
    { }

    /** 
     * Initializes this exemption mechanism with a key and a set of algorithm
     * parameters.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * and <code>params</code> is null, the underlying exemption
     * mechanism implementation is supposed to generate the required
     * parameters itself (using provider-specific default values); in the case
     * that algorithm parameters must be specified by the caller, an
     * <code>InvalidAlgorithmParameterException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
    public final void init(Key key, AlgorithmParameterSpec params)
        throws InvalidKeyException, InvalidAlgorithmParameterException,
        ExemptionMechanismException
    { }

    /** 
     * Initializes this exemption mechanism with a key and a set of algorithm
     * parameters.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * and <code>params</code> is null, the underlying exemption mechanism
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default values); in the case that algorithm
     * parameters must be specified by the caller, an
     * <code>InvalidAlgorithmParameterException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
    public final void init(Key key, AlgorithmParameters params)
        throws InvalidKeyException, InvalidAlgorithmParameterException,
        ExemptionMechanismException
    { }

    /** 
     * Generates the exemption mechanism key blob.
     *
     * @return the new buffer with the result key blob.
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final byte[] genExemptionBlob()
        throws IllegalStateException, ExemptionMechanismException
    {
        return null;
    }

    /** 
     * Generates the exemption mechanism key blob, and stores the result in
     * the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final int genExemptionBlob(byte[] output)
        throws IllegalStateException, ShortBufferException,
        ExemptionMechanismException
    {
        return 0;
    }

    /** 
     * Generates the exemption mechanism key blob, and stores the result in
     * the <code>output</code> buffer, starting at <code>outputOffset</code>
     * inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final int genExemptionBlob(byte[] output, int outputOffset)
        throws IllegalStateException, ShortBufferException,
        ExemptionMechanismException
    {
        return 0;
    }

    /** 
     * Ensures that the key stored away by this ExemptionMechanism
     * object will be wiped out when there are no more references to it.
     */
    protected void finalize() { }
}
