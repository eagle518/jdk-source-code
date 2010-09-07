/*
 * @(#)SealedObject.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.io.*;

import java.security.AlgorithmParameters;
import java.security.Key;
import java.security.InvalidKeyException;
import java.security.InvalidAlgorithmParameterException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;

/** 
 * This class enables a programmer to create an object and protect its
 * confidentiality with a cryptographic algorithm.
 *
 * <p> Given any Serializable object, one can create a SealedObject
 * that encapsulates the original object, in serialized
 * format (i.e., a "deep copy"), and seals (encrypts) its serialized contents,
 * using a cryptographic algorithm such as DES, to protect its
 * confidentiality.  The encrypted content can later be decrypted (with
 * the corresponding algorithm using the correct decryption key) and
 * de-serialized, yielding the original object.
 *
 * <p> Note that the Cipher object must be fully initialized with the
 * correct algorithm, key, padding scheme, etc., before being applied
 * to a SealedObject.
 *
 * <p> The original object that was sealed can be recovered in two different
 * ways: <p>
 *
 * <ul>
 *
 * <li>by using the {@link #getObject(javax.crypto.Cipher) getObject}
 * method that takes a <code>Cipher</code> object.
 * 
 * <p> This method requires a fully initialized <code>Cipher</code> object,
 * initialized with the
 * exact same algorithm, key, padding scheme, etc., that were used to seal the
 * object.
 *
 * <p> This approach has the advantage that the party who unseals the
 * sealed object does not require knowledge of the decryption key. For example,
 * after one party has initialized the cipher object with the required
 * decryption key, it could hand over the cipher object to
 * another party who then unseals the sealed object.
 *
 * <p>
 * 
 * <li>by using one of the
 * {@link #getObject(java.security.Key) getObject} methods
 * that take a <code>Key</code> object.
 *
 * <p> In this approach, the <code>getObject</code> method creates a cipher
 * object for the appropriate decryption algorithm and initializes it with the
 * given decryption key and the algorithm parameters (if any) that were stored
 * in the sealed object.
 *
 * <p> This approach has the advantage that the party who
 * unseals the object does not need to keep track of the parameters (e.g., an
 * IV) that were used to seal the object.
 *
 * </ul>
 *
 * @author Li Gong
 * @author Jan Luehe
 * @version 	1.30, 10/29/03
 * @see Cipher
 * @since 1.4
 */
public class SealedObject implements Serializable
{
    /** 
     * The encoded parameters.
     *
     * @serial
     */
    protected byte[] encodedParams;

    /** 
     * The serialized object contents in encrypted format.
     *
     * @serial
     */
    private byte[] encryptedContent;

    /** 
     * The algorithm that was used to seal this object.
     *
     * @serial
     */
    private String sealAlg;

    /** 
     * The algorithm of the parameters used.
     *
     * @serial
     */
    private String paramsAlg;

    /** 
     * Constructs a SealedObject from any Serializable object.
     *
     * <p>The given object is serialized, and its serialized contents are
     * encrypted using the given Cipher, which must be fully initialized.
     *
     * <p>Any algorithm parameters that may be used in the encryption
     * operation are stored inside of the new <code>SealedObject</code>.
     *
     * @param object the object to be sealed; can be null.
     * @param c the cipher used to seal the object.
     *
     * @exception NullPointerException if the given cipher is null.
     * @exception IOException if an error occurs during serialization
     * @exception IllegalBlockSizeException if the given cipher is a block
     * cipher, no padding has been requested, and the total input length
     * (i.e., the length of the serialized object contents) is not a multiple
     * of the cipher's block size
     */
    public SealedObject(Serializable object, Cipher c)
        throws IOException, IllegalBlockSizeException
    { }

    /** 
     * Constructs a SealedObject object from the passed-in SealedObject.
     *
     * @param so a SealedObject object
     * @exception NullPointerException if the given sealed object is null.
     */
    protected SealedObject(SealedObject so) { }

    /** 
     * Returns the algorithm that was used to seal this object.
     *
     * @return the algorithm that was used to seal this object.
     */
    public final String getAlgorithm() { }

    /** 
     * Retrieves the original (encapsulated) object.
     *
     * <p>This method creates a cipher for the algorithm that had been used in
     * the sealing operation.
     * If the default provider package provides an implementation of that
     * algorithm, an instance of Cipher containing that implementation is used.
     * If the algorithm is not available in the default package, other
     * packages are searched.
     * The Cipher object is initialized for decryption, using the given
     * <code>key</code> and the parameters (if any) that had been used in the
     * sealing operation.
     *
     * <p>The encapsulated object is unsealed and de-serialized, before it is
     * returned.
     *
     * @param key the key used to unseal the object.
     *
     * @return the original object.
     *
     * @exception IOException if an error occurs during de-serialiazation.
     * @exception ClassNotFoundException if an error occurs during 
     * de-serialiazation.
     * @exception NoSuchAlgorithmException if the algorithm to unseal the
     * object is not available.
     * @exception InvalidKeyException if the given key cannot be used to unseal
     * the object (e.g., it has the wrong algorithm).
     */
    public final Object getObject(Key key)
        throws IOException, ClassNotFoundException, NoSuchAlgorithmException,
        InvalidKeyException
    { }

    /** 
     * Retrieves the original (encapsulated) object.
     *
     * <p>The encapsulated object is unsealed (using the given Cipher,
     * assuming that the Cipher is already properly initialized) and
     * de-serialized, before it is returned.
     *
     * @param c the cipher used to unseal the object
     *
     * @return the original object.
     *
     * @exception NullPointerException if the given cipher is null.
     * @exception IOException if an error occurs during de-serialiazation
     * @exception ClassNotFoundException if an error occurs during 
     * de-serialiazation
     * @exception IllegalBlockSizeException if the given cipher is a block
     * cipher, no padding has been requested, and the total input length is
     * not a multiple of the cipher's block size
     * @exception BadPaddingException if the given cipher has been 
     * initialized for decryption, and padding has been specified, but
     * the input data does not have proper expected padding bytes
     */
    public final Object getObject(Cipher c)
        throws IOException, ClassNotFoundException, IllegalBlockSizeException,
        BadPaddingException
    { }

    /** 
     * Retrieves the original (encapsulated) object.
     *
     * <p>This method creates a cipher for the algorithm that had been used in
     * the sealing operation, using an implementation of that algorithm from
     * the given <code>provider</code>.
     * The Cipher object is initialized for decryption, using the given
     * <code>key</code> and the parameters (if any) that had been used in the
     * sealing operation.
     *
     * <p>The encapsulated object is unsealed and de-serialized, before it is
     * returned.
     *
     * @param key the key used to unseal the object.
     * @param provider the name of the provider of the algorithm to unseal
     * the object.
     *
     * @return the original object.
     *
     * @exception IllegalArgumentException if the given provider is null
     * or empty.
     * @exception IOException if an error occurs during de-serialiazation.
     * @exception ClassNotFoundException if an error occurs during 
     * de-serialiazation.
     * @exception NoSuchAlgorithmException if the algorithm to unseal the
     * object is not available.
     * @exception NoSuchProviderException if the given provider is not
     * configured.
     * @exception InvalidKeyException if the given key cannot be used to unseal
     * the object (e.g., it has the wrong algorithm).
     */
    public final Object getObject(Key key, String provider)
        throws IOException, ClassNotFoundException, NoSuchAlgorithmException,
        NoSuchProviderException, InvalidKeyException
    { }

    /** 
     * Restores the state of the SealedObject from a stream.
     * @param s the object input stream.
     * @exception NullPointerException if s is null.
     */
    private void readObject(ObjectInputStream s)
        throws IOException, ClassNotFoundException
    { }
}
