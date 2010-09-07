/*
 * @(#)Cipher.java	1.13 04/03/31
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
import java.util.regex.*;
import java.security.*;
import javax.crypto.spec.*;
import sun.security.jca.*;

import java.security.Provider.Service;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.nio.ByteBuffer;
import java.nio.ReadOnlyBufferException;
import sun.security.util.Debug;
import sun.security.jca.GetInstance.Instance;

/** 
 * This class provides the functionality of a cryptographic cipher for
 * encryption and decryption. It forms the core of the Java Cryptographic
 * Extension (JCE) framework.
 *
 * <p>In order to create a Cipher object, the application calls the
 * Cipher's <code>getInstance</code> method, and passes the name of the
 * requested <i>transformation</i> to it. Optionally, the name of a provider
 * may be specified.
 *
 * <p>A <i>transformation</i> is a string that describes the operation (or
 * set of operations) to be performed on the given input, to produce some
 * output. A transformation always includes the name of a cryptographic
 * algorithm (e.g., <i>DES</i>), and may be followed by a feedback mode and
 * padding scheme.
 *
 * <p> A transformation is of the form:<p>
 *
 * <ul>
 * <li>"<i>algorithm/mode/padding</i>" or
 * <p>
 * <li>"<i>algorithm</i>"
 * </ul>
 *
 * <P> (in the latter case,
 * provider-specific default values for the mode and padding scheme are used).
 * For example, the following is a valid transformation:<p>
 *
 * <pre>
 *     Cipher c = Cipher.getInstance("<i>DES/CBC/PKCS5Padding</i>");
 * </pre>
 *
 * <p>When requesting a block cipher in stream cipher mode (e.g.,
 * <code>DES</code> in <code>CFB</code> or <code>OFB</code> mode), the user may
 * optionally specify the number of bits to be
 * processed at a time, by appending this number to the mode name as shown in
 * the "<i>DES/CFB8/NoPadding</i>" and "<i>DES/OFB32/PKCS5Padding</i>"
 * transformations. If no such number is specified, a provider-specific default
 * is used. (For example, the "SunJCE" provider uses a default of 64 bits.)
 * 
 * @author Jan Luehe
 *
 * @version 1.123, 03/31/04
 *
 * @see KeyGenerator
 * @see SecretKey
 * @since 1.4
 */
public class Cipher
{
    /** 
     * Constant used to initialize cipher to encryption mode.
     */
    public static final int ENCRYPT_MODE = 1;

    /** 
     * Constant used to initialize cipher to decryption mode.
     */
    public static final int DECRYPT_MODE = 2;

    /** 
     * Constant used to initialize cipher to key-wrapping mode.
     */
    public static final int WRAP_MODE = 3;

    /** 
     * Constant used to initialize cipher to key-unwrapping mode.
     */
    public static final int UNWRAP_MODE = 4;

    /** 
     * Constant used to indicate the to-be-unwrapped key is a "public key".
     */
    public static final int PUBLIC_KEY = 1;

    /** 
     * Constant used to indicate the to-be-unwrapped key is a "private key". 
     */
    public static final int PRIVATE_KEY = 2;

    /** 
     * Constant used to indicate the to-be-unwrapped key is a "secret key".
     */
    public static final int SECRET_KEY = 3;

    /** 
     * Creates a Cipher object.
     *
     * @param cipherSpi the delegate
     * @param provider the provider
     * @param transformation the transformation
     */
    protected Cipher(CipherSpi cipherSpi, Provider provider, String
        transformation)
    { }

    /** 
     * Generates a <code>Cipher</code> object that implements the specified
     * transformation.
     *
     * <p>If the default provider package supplies an implementation of the
     * requested transformation, an instance of <code>Cipher</code> containing
     * that implementation is returned.
     * If the transformation is not available in the default provider package,
     * other provider packages are searched.
     *
     * @param transformation the name of the transformation, e.g.,
     * <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if <code>transformation</code> 
     * is null, empty, in an invalid format, or not available from the 
     * current installed providers.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     */
    public static final Cipher getInstance(String transformation)
        throws NoSuchAlgorithmException, NoSuchPaddingException
    { }

    /** 
     * Creates a <code>Cipher</code> object that implements the specified
     * transformation, as supplied by the specified provider.
     *
     * @param transformation the name of the transformation,
     * e.g., <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     * @param provider the name of the provider
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if <code>transformation</code> 
     * is null, empty, in an invalid format, or not available from the  
     * specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Cipher getInstance(String transformation, String
        provider)
        throws NoSuchAlgorithmException, NoSuchProviderException,
        NoSuchPaddingException
    { }

    /** 
     * Creates a <code>Cipher</code> object that implements the specified
     * transformation, as supplied by the specified provider. Note: the 
     * <code>provider</code> doesn't have to be registered.
     *
     * @param transformation the name of the transformation,
     * e.g., <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     * @param provider the provider
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if <code>transformation</code>
     * is null, empty, in an invalid format, or not available from the
     * specified provider.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Cipher getInstance(String transformation, Provider
        provider) throws NoSuchAlgorithmException, NoSuchPaddingException
    { }

    /** 
     * Returns the provider of this <code>Cipher</code> object.
     * 
     * @return the provider of this <code>Cipher</code> object
     */
    public final Provider getProvider() { }

    /** 
     * Returns the algorithm name of this <code>Cipher</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this <code>Cipher</code>
     * object..
     *
     * @return the algorithm name of this <code>Cipher</code> object. 
     */
    public final String getAlgorithm() { }

    /** 
     * Returns the block size (in bytes).
     *
     * @return the block size (in bytes), or 0 if the underlying algorithm is
     * not a block cipher
     */
    public final int getBlockSize() { }

    /** 
     * Returns the length in bytes that an output buffer would need to be in
     * order to hold the result of the next <code>update</code> or
     * <code>doFinal</code> operation, given the input length
     * <code>inputLen</code> (in bytes).
     *
     * <p>This call takes into account any unprocessed (buffered) data from a
     * previous <code>update</code> call, and padding.
     *
     * <p>The actual output length of the next <code>update</code> or
     * <code>doFinal</code> call may be smaller than the length returned by
     * this method.
     *
     * @param inputLen the input length (in bytes)
     *
     * @return the required output buffer size (in bytes)
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not yet been initialized)
     */
    public final int getOutputSize(int inputLen) { }

    /** 
     * Returns the initialization vector (IV) in a new buffer.
     *
     * <p>This is useful in the case where a random IV was created,
     * or in the context of password-based encryption or
     * decryption, where the IV is derived from a user-supplied password. 
     *
     * @return the initialization vector in a new buffer, or null if the
     * underlying algorithm does not use an IV, or if the IV has not yet
     * been set.
     */
    public final byte[] getIV() { }

    /** 
     * Returns the parameters used with this cipher.
     *
     * <p>The returned parameters may be the same that were used to initialize
     * this cipher, or may contain a combination of default and random
     * parameter values used by the underlying cipher implementation if this
     * cipher requires algorithm parameters but was not initialized with any.
     *
     * @return the parameters used with this cipher, or null if this cipher
     * does not use any parameters.
     */
    public final AlgorithmParameters getParameters() { }

    /** 
     * Returns the exemption mechanism object used with this cipher.
     *
     * @return the exemption mechanism object used with this cipher, or
     * null if this cipher does not use any exemption mechanism.
     */
    public final ExemptionMechanism getExemptionMechanism() { }

    /** 
     * Initializes this cipher with a key.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the given <code>key</code>, the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of
     * the following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the key
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or if this cipher is being initialized for
     * decryption and requires algorithm parameters that cannot be
     * determined from the given key, or if the given key has a keysize that
     * exceeds the maximum allowable keysize (as determined from the
     * configured jurisdiction policy files).
     */
    public final void init(int opmode, Key key) throws InvalidKeyException { }

    /** 
     * Initializes this cipher with a key and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the given <code>key</code>, the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or if this cipher is being initialized for
     * decryption and requires algorithm parameters that cannot be
     * determined from the given key, or if the given key has a keysize that
     * exceeds the maximum allowable keysize (as determined from the
     * configured jurisdiction policy files).
     */
    public final void init(int opmode, Key key, SecureRandom random)
        throws InvalidKeyException
    { }

    /** 
     * Initializes this cipher with a key and a set of algorithm
     * parameters.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameterSpec params)
        throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this cipher with a key, a set of algorithm
     * parameters, and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameterSpec params,
        SecureRandom random)
        throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this cipher with a key and a set of algorithm
     * parameters.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following: <code>ENCRYPT_MODE</code>,
     * <code>DECRYPT_MODE</code>, <code>WRAP_MODE</code>
     * or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameters params)
        throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this cipher with a key, a set of algorithm
     * parameters, and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following: <code>ENCRYPT_MODE</code>, 
     * <code>DECRYPT_MODE</code>, <code>WRAP_MODE</code>
     * or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameters params,
        SecureRandom random)
        throws InvalidKeyException, InvalidAlgorithmParameterException
    { }

    /** 
     * Initializes this cipher with the public key from the given certificate.
     * <p> The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If the certificate is of type X.509 and has a <i>key usage</i>
     * extension field marked as critical, and the value of the <i>key usage</i>
     * extension field implies that the public key in
     * the certificate and its corresponding private key are not
     * supposed to be used for the operation represented by the value 
     * of <code>opmode</code>,
     * an <code>InvalidKeyException</code>
     * is thrown.
     *
     * <p> If this cipher requires any algorithm parameters that cannot be
     * derived from the public key in the given certificate, the underlying 
     * cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or ramdom values) if it is being
     * initialized for encryption or key wrapping, and raise an <code>
     * InvalidKeyException</code> if it is being initialized for decryption or 
     * key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the
     * <code>SecureRandom</code>
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param certificate the certificate
     *
     * @exception InvalidKeyException if the public key in the given
     * certificate is inappropriate for initializing this cipher, or this
     * cipher is being initialized for decryption or unwrapping keys and
     * requires algorithm parameters that cannot be determined from the
     * public key in the given certificate, or the keysize of the public key
     * in the given certificate has a keysize that exceeds the maximum
     * allowable keysize (as determined by the configured jurisdiction policy
     * files).
     */
    public final void init(int opmode, Certificate certificate)
        throws InvalidKeyException
    { }

    /** 
     * Initializes this cipher with the public key from the given certificate
     * and 
     * a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping
     * or key unwrapping, depending on
     * the value of <code>opmode</code>.
     *
     * <p>If the certificate is of type X.509 and has a <i>key usage</i>
     * extension field marked as critical, and the value of the <i>key usage</i>
     * extension field implies that the public key in
     * the certificate and its corresponding private key are not
     * supposed to be used for the operation represented by the value of
     * <code>opmode</code>,
     * an <code>InvalidKeyException</code>
     * is thrown.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the public key in the given <code>certificate</code>,
     * the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param certificate the certificate
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the public key in the given
     * certificate is inappropriate for initializing this cipher, or this
     * cipher is being initialized for decryption or unwrapping keys and
     * requires algorithm parameters that cannot be determined from the
     * public key in the given certificate, or the keysize of the public key
     * in the given certificate has a keysize that exceeds the maximum
     * allowable keysize (as determined by the configured jurisdiction policy
     * files).
     */
    public final void init(int opmode, Certificate certificate, SecureRandom
        random) throws InvalidKeyException
    { }

    /** 
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The bytes in the <code>input</code> buffer are processed, and the
     * result is stored in a new buffer.
     *
     * <p>If <code>input</code> has a length of zero, this method returns
     * <code>null</code>.
     *
     * @param input the input buffer
     *
     * @return the new buffer with the result, or null if the underlying
     * cipher is a block cipher and the input data is too short to result in a
     * new block.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     */
    public final byte[] update(byte[] input) { }

    /** 
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in a new buffer.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * <code>null</code>.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     *
     * @return the new buffer with the result, or null if the underlying
     * cipher is a block cipher and the input data is too short to result in a
     * new block.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     */
    public final byte[] update(byte[] input, int inputOffset, int inputLen) { }

    /** 
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * a length of zero.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     */
    public final int update(byte[] input, int inputOffset, int inputLen, byte[]
        output) throws ShortBufferException
    { }

    /** 
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * a length of zero.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     */
    public final int update(byte[] input, int inputOffset, int inputLen, byte[]
        output, int outputOffset) throws ShortBufferException
    { }

    /** 
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>All <code>input.remaining()</code> bytes starting at
     * <code>input.position()</code> are processed. The result is stored
     * in the output buffer.
     * Upon return, the input buffer's position will be equal
     * to its limit; its limit will not have changed. The output buffer's
     * position will have advanced by n, where n is the value returned
     * by this method; the output buffer's limit will not have changed.
     *
     * <p>If <code>output.remaining()</code> bytes are insufficient to
     * hold the result, a <code>ShortBufferException</code> is thrown.
     * In this case, repeat this call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same block of memory and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input ByteBuffer
     * @param output the output ByteByffer
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalArgumentException if input and output are the
     *   same object
     * @exception ReadOnlyBufferException if the output buffer is read-only
     * @exception ShortBufferException if there is insufficient space in the
     * output buffer
     * @since 1.5
     */
    public final int update(ByteBuffer input, ByteBuffer output)
        throws ShortBufferException
    { }

    /** 
     * Finishes a multiple-part encryption or decryption operation, depending
     * on how this cipher was initialized.
     *
     * <p>Input data that may have been buffered during a previous
     * <code>update</code> operation is processed, with padding (if requested)
     * being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again. 
     * 
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal()
        throws IllegalBlockSizeException, BadPaddingException
    { }

    /** 
     * Finishes a multiple-part encryption or decryption operation, depending
     * on how this cipher was initialized.
     *
     * <p>Input data that may have been buffered during a previous
     * <code>update</code> operation is processed, with padding (if requested)
     * being applied.
     * The result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final int doFinal(byte[] output, int outputOffset)
        throws IllegalBlockSizeException, ShortBufferException,
        BadPaddingException
    { }

    /** 
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The bytes in the <code>input</code> buffer, and any input bytes that
     * may have been buffered during a previous <code>update</code> operation,
     * are processed, with padding (if requested) being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param input the input buffer
     *
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal(byte[] input)
        throws IllegalBlockSizeException, BadPaddingException
    { }

    /** 
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous <code>update</code>
     * operation, are processed, with padding (if requested) being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     *
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal(byte[] input, int inputOffset, int inputLen)
        throws IllegalBlockSizeException, BadPaddingException
    { }

    /** 
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous <code>update</code>
     * operation, are processed, with padding (if requested) being applied.
     * The result is stored in the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data.
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final int doFinal(byte[] input, int inputOffset, int inputLen, byte[]
        output)
        throws ShortBufferException, IllegalBlockSizeException,
        BadPaddingException
    { }

    /** 
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous
     * <code>update</code> operation, are processed, with padding
     * (if requested) being applied.
     * The result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data.
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final int doFinal(byte[] input, int inputOffset, int inputLen, byte[]
        output, int outputOffset)
        throws ShortBufferException, IllegalBlockSizeException,
        BadPaddingException
    { }

    /** 
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>All <code>input.remaining()</code> bytes starting at
     * <code>input.position()</code> are processed. The result is stored
     * in the output buffer.
     * Upon return, the input buffer's position will be equal
     * to its limit; its limit will not have changed. The output buffer's
     * position will have advanced by n, where n is the value returned
     * by this method; the output buffer's limit will not have changed.
     *
     * <p>If <code>output.remaining()</code> bytes are insufficient to
     * hold the result, a <code>ShortBufferException</code> is thrown.
     * In this case, repeat this call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data.
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input ByteBuffer
     * @param output the output ByteBuffer
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalArgumentException if input and output are the
     *   same object
     * @exception ReadOnlyBufferException if the output buffer is read-only
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size; or if this encryption algorithm is unable to
     * process the input data provided.
     * @exception ShortBufferException if there is insufficient space in the
     * output buffer
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     * @since 1.5
     */
    public final int doFinal(ByteBuffer input, ByteBuffer output)
        throws ShortBufferException, IllegalBlockSizeException,
        BadPaddingException
    { }

    /** 
     * Wrap a key.
     *
     * @param key the key to be wrapped.
     *
     * @return the wrapped key.
     *
     * @exception IllegalStateException if this cipher is in a wrong
     * state (e.g., has not been initialized).
     *
     * @exception IllegalBlockSizeException if this cipher is a block 
     * cipher, no padding has been requested, and the length of the 
     * encoding of the key to be wrapped is not a
     * multiple of the block size.
     *
     * @exception InvalidKeyException if it is impossible or unsafe to
     * wrap the key with this cipher (e.g., a hardware protected key is 
     * being passed to a software-only cipher).
     */
    public final byte[] wrap(Key key)
        throws IllegalBlockSizeException, InvalidKeyException
    { }

    /** 
     * Unwrap a previously wrapped key.
     *
     * @param wrappedKey the key to be unwrapped.
     *
     * @param wrappedKeyAlgorithm the algorithm associated with the wrapped
     * key.
     *
     * @param wrappedKeyType the type of the wrapped key. This must be one of
     * <code>SECRET_KEY</code>, <code>PRIVATE_KEY</code>, or
     * <code>PUBLIC_KEY</code>.
     *
     * @return the unwrapped key.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized).
     *
     * @exception NoSuchAlgorithmException if no installed providers
     * can create keys of type <code>wrappedKeyType</code> for the
     * <code>wrappedKeyAlgorithm</code>.
     *
     * @exception InvalidKeyException if <code>wrappedKey</code> does not
     * represent a wrapped key of type <code>wrappedKeyType</code> for
     * the <code>wrappedKeyAlgorithm</code>.
     */
    public final Key unwrap(byte[] wrappedKey, String wrappedKeyAlgorithm, int
        wrappedKeyType) throws InvalidKeyException, NoSuchAlgorithmException
    { }

    /** 
     * Returns the maximum key length for the specified transformation
     * according to the installed JCE jurisdiction policy files. If 
     * JCE unlimited strength jurisdiction policy files are installed, 
     * Integer.MAX_VALUE will be returned.
     * For more information on default key size in JCE jurisdiction 
     * policy files, please see appendix E in 
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppE">
     * JCE Reference Guide</a>.
     *
     * @param transformation the cipher transformation.
     * @return the maximum key length in bits or Integer.MAX_VALUE.
     * @exception NullPointerException if <code>transformation</code> is null.
     * @exception NoSuchAlgorithmException if <code>transformation</code> 
     * is not a valid transformation, i.e. in the form of "algorithm" or
     * "algorithm/mode/padding".
     * @since 1.5
     */
    public static final int getMaxAllowedKeyLength(String transformation)
        throws NoSuchAlgorithmException
    { }

    /** 
     * Returns an AlgorithmParameterSpec object which contains
     * the maximum cipher parameter value according to the
     * jurisdiction policy file. If JCE unlimited strength jurisdiction
     * policy files are installed or there is no maximum limit on the
     * parameters for the specified transformation in the policy file, 
     * null will be returned.
     *
     * @param transformation the cipher transformation.
     * @return an AlgorithmParameterSpec which holds the maximum
     * value or null.
     * @exception NullPointerException if <code>transformation</code> 
     * is null.
     * @exception NoSuchAlgorithmException if <code>transformation</code> 
     * is not a valid transformation, i.e. in the form of "algorithm" or
     * "algorithm/mode/padding".
     * @since 1.5
     */
    public static final AlgorithmParameterSpec getMaxAllowedParameterSpec(String
        transformation) throws NoSuchAlgorithmException
    { }
}
