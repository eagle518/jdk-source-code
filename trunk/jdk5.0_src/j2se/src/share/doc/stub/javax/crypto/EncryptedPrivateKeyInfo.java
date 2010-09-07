/*
 * @(#)EncryptedPrivateKeyInfo.java	1.13 04/01/14
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

import java.io.*;
import java.security.*;
import java.security.spec.*;

import sun.security.x509.AlgorithmId;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;

/** 
 * This class implements the <code>EncryptedPrivateKeyInfo</code> type
 * as defined in PKCS #8.
 * <p>Its ASN.1 definition is as follows:
 *
 * <pre>
 * EncryptedPrivateKeyInfo ::=  SEQUENCE {
 *     encryptionAlgorithm   AlgorithmIdentifier,
 *     encryptedData   OCTET STRING }
 * 
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm              OBJECT IDENTIFIER,
 *     parameters             ANY DEFINED BY algorithm OPTIONAL  }
 * </pre>
 *
 * @author Valerie Peng
 *
 * @version 1.14, 04/01/14
 * 
 * @see java.security.spec.PKCS8EncodedKeySpec
 *
 * @since 1.4
 */
public class EncryptedPrivateKeyInfo
{

    /** 
     * Constructs (i.e., parses) an <code>EncryptedPrivateKeyInfo</code> from
     * its ASN.1 encoding.
     * @param encoded the ASN.1 encoding of this object. The contents of 
     * the array are copied to protect against subsequent modification.
     * @exception NullPointerException if the <code>encoded</code> is null.
     * @exception IOException if error occurs when parsing the ASN.1 encoding.
     */
    public EncryptedPrivateKeyInfo(byte[] encoded) throws IOException { }

    /** 
     * Constructs an <code>EncryptedPrivateKeyInfo</code> from the
     * encryption algorithm name and the encrypted data.
     *
     * <p>Note: This constructor will use null as the value of the 
     * algorithm parameters. If the encryption algorithm has 
     * parameters whose value is not null, a different constructor,
     * e.g. EncryptedPrivateKeyInfo(AlgorithmParameters, byte[]), 
     * should be used.
     *
     * @param algName encryption algorithm name. See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a>
     * for information about standard Cipher algorithm names.
     * @param encryptedData encrypted data. The contents of 
     * <code>encrypedData</code> are copied to protect against subsequent 
     * modification when constructing this object.
     * @exception NullPointerException if <code>algName</code> or
     * <code>encryptedData</code> is null.
     * @exception IllegalArgumentException if <code>encryptedData</code>
     * is empty, i.e. 0-length.
     * @exception NoSuchAlgorithmException if the specified algName is
     * not supported.
     */
    public EncryptedPrivateKeyInfo(String algName, byte[] encryptedData)
        throws NoSuchAlgorithmException
    { }

    /** 
     * Constructs an <code>EncryptedPrivateKeyInfo</code> from the
     * encryption algorithm parameters and the encrypted data.
     *
     * @param algParams the algorithm parameters for the encryption 
     * algorithm. <code>algParams.getEncoded()</code> should return
     * the ASN.1 encoded bytes of the <code>parameters</code> field
     * of the <code>AlgorithmIdentifer</code> component of the
     * <code>EncryptedPrivateKeyInfo</code> type. 
     * @param encryptedData encrypted data. The contents of
     * <code>encrypedData</code> are copied to protect against 
     * subsequent modification when constructing this object.
     * @exception NullPointerException if <code>algParams</code> or 
     * <code>encryptedData</code> is null.
     * @exception IllegalArgumentException if <code>encryptedData</code>
     * is empty, i.e. 0-length.
     * @exception NoSuchAlgorithmException if the specified algName of
     * the specified <code>algParams</code> parameter is not supported.
     */
    public EncryptedPrivateKeyInfo(AlgorithmParameters algParams, byte[]
        encryptedData) throws NoSuchAlgorithmException
    { }

    /** 
     * Returns the encryption algorithm.
     * <p>Note: Standard name is returned instead of the specified one
     * in the constructor when such mapping is available. 
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a>
     * for information about standard Cipher algorithm names.
     * 
     * @return the encryption algorithm name.	
     */
    public String getAlgName() { }

    /** 
     * Returns the algorithm parameters used by the encryption algorithm.
     * @return the algorithm parameters.
     */
    public AlgorithmParameters getAlgParameters() { }

    /** 
     * Returns the encrypted data.
     * @return the encrypted data. Returns a new array
     * each time this method is called.
     */
    public byte[] getEncryptedData() { }

    /** 
     * Extract the enclosed PKCS8EncodedKeySpec object from the 
     * encrypted data and return it.
     * <br>Note: In order to successfully retrieve the enclosed
     * PKCS8EncodedKeySpec object, <code>cipher</code> needs
     * to be initialized to either Cipher.DECRYPT_MODE or 
     * Cipher.UNWRAP_MODE, with the same key and parameters used 
     * for generating the encrypted data. 
     *
     * @param cipher the initialized cipher object which will be
     * used for decrypting the encrypted data.
     * @return the PKCS8EncodedKeySpec object. 
     * @exception NullPointerException if <code>cipher</code>
     * is null.
     * @exception InvalidKeySpecException if the given cipher is 
     * inappropriate for the encrypted data or the encrypted
     * data is corrupted and cannot be decrypted.
     */
    public PKCS8EncodedKeySpec getKeySpec(Cipher cipher)
        throws InvalidKeySpecException
    { }

    /** 
     * Extract the enclosed PKCS8EncodedKeySpec object from the
     * encrypted data and return it.
     * @param decryptKey key used for decrypting the encrypted data.
     * @return the PKCS8EncodedKeySpec object.
     * @exception NullPointerException if <code>decryptKey</code>
     * is null.
     * @exception NoSuchAlgorithmException if cannot find appropriate 
     * cipher to decrypt the encrypted data.
     * @exception InvalidKeyException if <code>decryptKey</code>
     * cannot be used to decrypt the encrypted data or the decryption 
     * result is not a valid PKCS8KeySpec.
     */
    public PKCS8EncodedKeySpec getKeySpec(Key decryptKey)
        throws NoSuchAlgorithmException, InvalidKeyException
    { }

    /** 
     * Extract the enclosed PKCS8EncodedKeySpec object from the
     * encrypted data and return it.
     * @param decryptKey key used for decrypting the encrypted data.
     * @param providerName the name of provider whose Cipher 
     * implementation will be used.
     * @return the PKCS8EncodedKeySpec object.
     * @exception NullPointerException if <code>decryptKey</code>
     * or <code>providerName</code> is null.
     * @exception NoSuchProviderException if no provider
     * <code>providerName</code> is registered.
     * @exception NoSuchAlgorithmException if cannot find appropriate 
     * cipher to decrypt the encrypted data.
     * @exception InvalidKeyException if <code>decryptKey</code>
     * cannot be used to decrypt the encrypted data or the decryption 
     * result is not a valid PKCS8KeySpec.
     */
    public PKCS8EncodedKeySpec getKeySpec(Key decryptKey, String providerName)
        throws NoSuchProviderException, NoSuchAlgorithmException,
        InvalidKeyException
    { }

    /** 
     * Extract the enclosed PKCS8EncodedKeySpec object from the
     * encrypted data and return it.
     * @param decryptKey key used for decrypting the encrypted data.
     * @param provider the name of provider whose Cipher implementation
     * will be used.
     * @return the PKCS8EncodedKeySpec object.
     * @exception NullPointerException if <code>decryptKey</code>
     * or <code>provider</code> is null.
     * @exception NoSuchAlgorithmException if cannot find appropriate 
     * cipher to decrypt the encrypted data in <code>provider</code>.
     * @exception InvalidKeyException if <code>decryptKey</code>
     * cannot be used to decrypt the encrypted data or the decryption
     * result is not a valid PKCS8KeySpec.
     */
    public PKCS8EncodedKeySpec getKeySpec(Key decryptKey, Provider provider)
        throws NoSuchAlgorithmException, InvalidKeyException
    { }

    /** 
     * Returns the ASN.1 encoding of this object.
     * @return the ASN.1 encoding. Returns a new array
     * each time this method is called.
     * @exception IOException if error occurs when constructing its
     * ASN.1 encoding.
     */
    public byte[] getEncoded() throws IOException { }
}
