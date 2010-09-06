/*
 * @(#)CipherHelper.java	1.1 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss.krb5;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import org.ietf.jgss.*;

import java.security.MessageDigest;
import java.security.GeneralSecurityException;
import java.security.NoSuchAlgorithmException;
import sun.security.krb5.*;
import sun.security.krb5.internal.crypto.Des3;

class CipherHelper {
    
    // From draft-raeburn-cat-gssapi-krb5-3des-00
    // Key usage values when deriving keys
    private static final int KG_USAGE_SEAL = 22;
    private static final int KG_USAGE_SIGN = 23;
    private static final int KG_USAGE_SEQ = 24;

    private static final int DES_CHECKSUM_SIZE = 8;
    private static final int IV_SIZE = 8;
    /**
     * A zero initial vector to be used for checksum calculation and for
     * DesCbc application data encryption/decryption.
     */
    private static final byte[] ZERO_IV = new byte[IV_SIZE];


    /* DESCipher instance used by the corresponding GSSContext */
    private Cipher desCipher = null;

    private int etype;
    private int sgnAlg, sealAlg;
    private byte[] keybytes;

    CipherHelper(EncryptionKey key) throws GSSException {
	etype = key.getEType();
	keybytes = key.getBytes();
	
	switch (etype) {
	case EncryptedData.ETYPE_DES_CBC_CRC:
	    sgnAlg = MessageToken.SGN_ALG_DES_MAC;
	    sealAlg = MessageToken.SEAL_ALG_DES;
	    getDesCipher();
	    break;

	case EncryptedData.ETYPE_DES_CBC_MD5:
	    sgnAlg = MessageToken.SGN_ALG_DES_MAC_MD5;
	    sealAlg = MessageToken.SEAL_ALG_DES;
	    getDesCipher();
	    break;

	case EncryptedData.ETYPE_DES3_CBC_HMAC_SHA1_KD:
	    sgnAlg = MessageToken.SGN_ALG_HMAC_SHA1_DES3_KD;
	    sealAlg = MessageToken.SEAL_ALG_DES3_KD;
	    break;

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported encryption type: " + etype);
	}
    }

    int getSgnAlg() {
	return sgnAlg;
    }

    int getSealAlg() {
	return sealAlg;
    }

    int getEType() {
	return etype;
    }

    byte[] calculateChecksum(int alg, byte[] header, byte[] trailer,
	byte[] data, int start, int len) throws GSSException {

	switch (alg) {
	case MessageToken.SGN_ALG_DES_MAC_MD5:
	    /*
	     * With this sign algorithm, first an MD5 hash is computed on the
	     * application data. The 16 byte hash is then DesCbc encrypted.
	     */
	    try {
		MessageDigest md5 = MessageDigest.getInstance("MD5");
          
		// debug("\t\tdata=[");
          
		// debug(getHexBytes(checksumDataHeader, 
	 	//			checksumDataHeader.length) + " ");
		md5.update(header);
          
		// debug(getHexBytes(data, start, len));
		md5.update(data, start, len);
          
		if (trailer != null) {
		    // debug(" " + 
		    // 	     getHexBytes(trailer, 
		    // 			   optionalTrailer.length));
		    md5.update(trailer);
		}
		//          debug("]\n");
          
		data = md5.digest();
		start = 0;
		len = data.length;
		//          System.out.println("\tMD5 Checksum is [" + 
		//                             getHexBytes(data) + "]\n");
		header = null;
		trailer = null;
	    } catch (NoSuchAlgorithmException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		    "Could not get MD5 Message Digest - " + e.getMessage());
		ge.initCause(e);
		throw ge;
	    }
	    // fall through to encrypt checksum

	case MessageToken.SGN_ALG_DES_MAC:
	    return getDesCbcChecksum(keybytes, header, data, start, len);

	case MessageToken.SGN_ALG_HMAC_SHA1_DES3_KD:
	    byte[] buf;
	    int offset, total;
	    if (header == null && trailer == null) {
	        buf = data;
	        total = len;
	        offset = start;
	    } else {
		total = ((header != null ? header.length : 0) + len +
		    (trailer != null ? trailer.length : 0));
		
	        buf = new byte[total];
		int pos = 0;
		if (header != null) {
		    System.arraycopy(header, 0, buf, 0, header.length);
		    pos = header.length;
		}
	        System.arraycopy(data, 0, buf, pos, len);
		pos += len;
		if (trailer != null) {
		    System.arraycopy(trailer, 0, buf, pos, trailer.length);
		}

	        offset = 0;
	    }
	    
	    try {
		/*
		Krb5Token.debug("\nkeybytes: " + 
		    Krb5Token.getHexBytes(keybytes));
		Krb5Token.debug("\nheader: " + (header == null ? "NONE" : 
		    Krb5Token.getHexBytes(header)));
		Krb5Token.debug("\ntrailer: " + (trailer == null ? "NONE" : 
		    Krb5Token.getHexBytes(trailer)));
		Krb5Token.debug("\ndata: " + 
		    Krb5Token.getHexBytes(data, start, len));
		Krb5Token.debug("\nbuf: " + Krb5Token.getHexBytes(buf, offset,
		    total));
		*/

		byte[] answer = Des3.calculateChecksum(keybytes, 
		    KG_USAGE_SIGN, buf, offset, total);

		// Krb5Token.debug("\nanswer: " + Krb5Token.getHexBytes(answer));

		return answer;
	    } catch (GeneralSecurityException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Could not use HMAC-SHA1-DES3-KD signing algorithm - " +
		    e.getMessage());
		ge.initCause(e);
		throw ge;
	    }

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported signing algorithm: " + sgnAlg);
	}
    }

    byte[] encryptSeq(byte[] ivec, byte[] plaintext, int start, int len) 
    throws GSSException {
	switch (sgnAlg) {
	case MessageToken.SGN_ALG_DES_MAC_MD5:
	case MessageToken.SGN_ALG_DES_MAC:
	    try {
		Cipher des = getInitializedDes(true, keybytes, ivec);
		return des.doFinal(plaintext, start, len);

	    } catch (GeneralSecurityException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Could not encrypt sequence number using DES - " +
		    e.getMessage());
		ge.initCause(e);
		throw ge;
	    }

	case MessageToken.SGN_ALG_HMAC_SHA1_DES3_KD:
	    byte[] iv;
	    if (ivec.length == IV_SIZE) {
		iv = ivec;
	    } else {
		iv = new byte[IV_SIZE];
		System.arraycopy(ivec, 0, iv, 0, IV_SIZE);
	    }
	    try {
		return Des3.encryptRaw(keybytes, KG_USAGE_SEQ, iv, 
		    plaintext, start, len);
	    } catch (Exception e) {
		// GeneralSecurityException, KrbCryptoException
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Could not encrypt sequence number using DES3-KD - " +
		    e.getMessage());
		ge.initCause(e);
		throw ge;
	    }

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported signing algorithm: " + sgnAlg);
	}
    }

    byte[] decryptSeq(byte[] ivec, byte[] ciphertext, int start, int len) 
        throws GSSException {

	switch (sgnAlg) {
	case MessageToken.SGN_ALG_DES_MAC_MD5:
	case MessageToken.SGN_ALG_DES_MAC:
	    try {
		Cipher des = getInitializedDes(false, keybytes, ivec); 
		return des.doFinal(ciphertext, start, len); 
	    } catch (GeneralSecurityException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Could not decrypt sequence number using DES - " +
		    e.getMessage());
		ge.initCause(e);
		throw ge;
	    }

	case MessageToken.SGN_ALG_HMAC_SHA1_DES3_KD:
	    byte[] iv;
	    if (ivec.length == IV_SIZE) {
		iv = ivec;
	    } else {
		iv = new byte[8];
		System.arraycopy(ivec, 0, iv, 0, IV_SIZE);
	    }

	    try {
		return Des3.decryptRaw(keybytes, KG_USAGE_SEQ, iv, 
		    ciphertext, start, len);
	    } catch (Exception e) {
		// GeneralSecurityException, KrbCryptoException
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Could not decrypt sequence number using DES3-KD - " +
		    e.getMessage());
		ge.initCause(e);
		throw ge;
	    }

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported signing algorithm: " + sgnAlg);
	}
    }

    int getChecksumLength() throws GSSException {
	switch (etype) {
	case EncryptedData.ETYPE_DES_CBC_CRC:
	case EncryptedData.ETYPE_DES_CBC_MD5:
	    return DES_CHECKSUM_SIZE;

	case EncryptedData.ETYPE_DES3_CBC_HMAC_SHA1_KD:
	    return Des3.getChecksumLength();

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported encryption type: "	+ etype);
	}
    }

    void decryptData(WrapToken token, byte[] ciphertext, int cStart, int cLen, 
	byte[] plaintext, int pStart) throws GSSException {

	switch (sealAlg) {
	case MessageToken.SEAL_ALG_DES:
	    desCbcDecrypt(token, getDesEncryptionKey(keybytes), 
		ciphertext, cStart, cLen, plaintext, pStart);
	    break;

	case MessageToken.SEAL_ALG_DES3_KD:
	    des3KdDecrypt(token, ciphertext, cStart, cLen, plaintext, pStart);
	    break;

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported seal algorithm: " + sealAlg);
	}
    }

    void decryptData(WrapToken token, InputStream cipherStream, int cLen, 
	byte[] plaintext, int pStart) throws GSSException, IOException {

	switch (sealAlg) {
	case MessageToken.SEAL_ALG_DES:
	    desCbcDecrypt(token, getDesEncryptionKey(keybytes), 
		cipherStream, cLen, plaintext, pStart);
	    break;

	case MessageToken.SEAL_ALG_DES3_KD:

	    // Read encrypted data from stream
	    byte[] ciphertext = new byte[cLen];
	    try {
		Krb5Token.readFully(cipherStream, ciphertext, 0, cLen);
	    } catch (IOException e) {
		GSSException ge = new GSSException(
		    GSSException.DEFECTIVE_TOKEN, -1, 
		    "Cannot read complete token");
		ge.initCause(e);
		throw ge;
	    }

	    des3KdDecrypt(token, ciphertext, 0, cLen, plaintext, pStart);
	    break;

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported seal algorithm: " + sealAlg);
	}
    }

    void encryptData(byte[] confounder, 
	byte[] plaintext, int start, int len, byte[] padding,
	OutputStream os) throws GSSException, IOException {

	switch (sealAlg) {
	case MessageToken.SEAL_ALG_DES:
	    // Encrypt on the fly and write
	    Cipher des = getInitializedDes(true, getDesEncryptionKey(keybytes),
		ZERO_IV);
	    CipherOutputStream cos = new CipherOutputStream(os, des);
	    //debug(getHexBytes(confounder, confounder.length));
	    cos.write(confounder);
	    //debug(" " + getHexBytes(plaintext, start, len));
	    cos.write(plaintext, start, len);
	    //debug(" " + getHexBytes(padding, padding.length));
	    cos.write(padding);
	    break;

	case MessageToken.SEAL_ALG_DES3_KD:
	    byte[] ctext = des3KdEncrypt(confounder, plaintext, start, len,
		padding);

	    // Write to stream
	    os.write(ctext);
	    break;

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported seal algorithm: " + sealAlg);
	}
    }

    void encryptData(byte[] confounder, byte[] plaintext, int pStart, 
	int pLen, byte[] padding, byte[] ciphertext, int cStart) 
	throws GSSException {

	switch (sealAlg) {
	case MessageToken.SEAL_ALG_DES:
	    int pos = cStart; 
	    // Encrypt and write
	    Cipher des = getInitializedDes(true, getDesEncryptionKey(keybytes),
		ZERO_IV);
	    try {
		//debug(getHexBytes(confounder, confounder.length));
		pos += des.update(confounder, 0, confounder.length,
				  ciphertext, pos);
		//debug(" " + getHexBytes(dataBytes, dataOffset, dataLen));
		pos += des.update(plaintext, pStart, pLen, 
				  ciphertext, pos);
		//debug(" " + getHexBytes(padding, padding.length));
		des.update(padding, 0, padding.length, 
			   ciphertext, pos);
		des.doFinal();
	    } catch (GeneralSecurityException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		    "Could not use DES Cipher - " + e.getMessage());
		ge.initCause(e);
		throw ge;
	    }
	    break;

	case MessageToken.SEAL_ALG_DES3_KD:
	    byte[] ctext = des3KdEncrypt(confounder, plaintext, pStart, pLen,
		padding);
	    System.arraycopy(ctext, 0, ciphertext, cStart, ctext.length);
	    break;

	default:
	    throw new GSSException(GSSException.FAILURE, -1,
		"Unsupported seal algorithm: " + sealAlg);
	}
    }

    // --------------------- DES methods

    private final Cipher getDesCipher() throws GSSException {
        if (desCipher == null) {
	    try {
		desCipher = Cipher.getInstance("DES/CBC/NoPadding");
	    } catch (GeneralSecurityException e) {
		GSSException ge = new GSSException(GSSException.FAILURE, -1,
		    "Cannot get DES cipher - " + e.getMessage());
		ge.initCause(e);
		throw ge;
	    }
	}
        return desCipher;
    }

    /**
     * Computes the DesCbc checksum based on the algorithm published in FIPS
     * Publication 113. This involves applying padding to the data passed
     * in, then performing DesCbc encryption on the data with a zero initial
     * vector, and finally returning the last 8 bytes of the encryption
     * result.
     *
     * @param key the bytes for the DES key
     * @param header a header to process first before the data is.
     * @param data the data to checksum
     * @param offset the offset where the data begins
     * @param len the length of the data
     * @throws GSSException when an error occuse in the encryption
     */
    private byte[] getDesCbcChecksum(byte key[],
				     byte[] header,
				     byte[] data, int offset, int len)
	throws GSSException {

	Cipher des = getInitializedDes(true, key, ZERO_IV);
        
	int blockSize = des.getBlockSize();
      
	/*
	 * Here the data need not be a multiple of the blocksize
	 * (8). Encrypt and throw away results for all blocks except for 
	 * the very last block.
	 */

	byte[] finalBlock = new byte[blockSize];
      
	int numBlocks = len / blockSize;
	int lastBytes = len % blockSize;
	if (lastBytes == 0) {
	    // No need for padding. Save last block from application data
	    numBlocks -= 1;
	    System.arraycopy(data, offset + numBlocks*blockSize, 
			     finalBlock, 0, blockSize);
	} else {
	    System.arraycopy(data, offset + numBlocks*blockSize,
			     finalBlock, 0, lastBytes);
	    // Zero padding automatically done
	}
      
	try {
	    byte[] temp = new byte[Math.max(blockSize, 
		(header == null? blockSize : header.length))];

	    if (header != null) {
		// header will be null when doing DES-MD5 Checksum
		des.update(header, 0, header.length, temp, 0);
	    }
        
	    // Iterate over all but the last block
	    for (int i = 0; i < numBlocks; i++) {
		des.update(data, offset, blockSize,
			   temp, 0);
		offset += blockSize;
	    }
        
	    // Now process the final block
	    byte[] retVal = new byte[blockSize];
	    des.update(finalBlock, 0, blockSize, retVal, 0);
	    des.doFinal();

	    return retVal;
	} catch (GeneralSecurityException e) {
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		"Could not use DES Cipher - " + e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}
    }
    
    /**
     * Obtains an initialized DES cipher.
     *
     * @param encryptMode true if encryption is desired, false is decryption
     * is desired.
     * @param key the bytes for the DES key
     * @param ivBytes the initial vector bytes
     */
    private final Cipher getInitializedDes(boolean encryptMode, byte[] key, 
					  byte[] ivBytes) 
	throws  GSSException  {
    
    
	try {
	    IvParameterSpec iv = new IvParameterSpec(ivBytes);
	    SecretKey jceKey = (SecretKey) (new SecretKeySpec(key, "DES")); 

	    if (desCipher == null) {
	        throw new GSSException(GSSException.FAILURE, -1, 
				"Internal Error:Uninitialized desCipher");
	    }
	    desCipher.init(
		(encryptMode ? Cipher.ENCRYPT_MODE : Cipher.DECRYPT_MODE),
		jceKey, iv);
	    return desCipher;
	} catch (GeneralSecurityException e) {
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}
    }

    /**
     * Helper routine to decrypt fromm a byte array and write the
     * application data straight to an output array with minimal
     * buffer copies. The confounder and the padding are stored
     * separately and not copied into this output array.
     * @param key the DES key to use
     * @param cipherText the encrypted data
     * @param offset the offset for the encrypted data
     * @param len the length of the encrypted data
     * @param dataOutBuf the output buffer where the application data 
     * should be writte
     * @param dataOffset the offser where the application data should 
     * be written.
     * @throws GSSException is an error occurs while decrypting the
     * data
     */
    private void desCbcDecrypt(WrapToken token, byte[] key, byte[] cipherText,
	int offset, int len, byte[] dataOutBuf, int dataOffset) 
	 throws GSSException {
	
	try {
	    
	    int temp = 0;
	    
	    Cipher des = getInitializedDes(false, key, ZERO_IV);

	    /*
	     * Remove the counfounder first.
	     * CONFOUNDER_SIZE is one DES block ie 8 bytes.
	     */
	    temp = des.update(cipherText, offset, WrapToken.CONFOUNDER_SIZE,
			      token.confounder);
	    // temp should be CONFOUNDER_SIZE
	    //debug("\n\ttemp is " + temp + " and CONFOUNDER_SIZE is " 
	    //  + CONFOUNDER_SIZE);

	    offset += WrapToken.CONFOUNDER_SIZE;
	    len -= WrapToken.CONFOUNDER_SIZE;
      
	    /*
	     * len is a multiple of 8 due to padding.
	     * Decrypt all blocks directly into the output buffer except for
	     * the very last block. Remove the trailing padding bytes from the
	     * very last block and copy that into the output buffer.
	     */
      
	    int blockSize = des.getBlockSize();
	    int numBlocks = len / blockSize - 1;
	    
	    // Iterate over all but the last block
	    for (int i = 0; i < numBlocks; i++) {
		temp = des.update(cipherText, offset, blockSize,
				  dataOutBuf, dataOffset);
		// temp should be blockSize
		//debug("\n\ttemp is " + temp + " and blockSize is " 
		//    + blockSize);

		offset += blockSize;
		dataOffset += blockSize;
	    }
      
	    // Now process the last block
	    byte[] finalBlock = new byte[blockSize];
	    des.update(cipherText, offset, blockSize, finalBlock);

	    des.doFinal();
      
	    /*
	     * There is always at least one padding byte. The padding bytes
	     * are all the value of the number of padding bytes.
	     */

	    int padSize = finalBlock[blockSize - 1];
	    if (padSize < 1  || padSize > 8)
		throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
					"Invalid padding on Wrap Token");
	    token.padding = WrapToken.pads[padSize];
	    blockSize -= padSize;
	    
	    // Copy this last block into the output buffer
	    System.arraycopy(finalBlock, 0, dataOutBuf, dataOffset,
			     blockSize);
	     
	} catch (GeneralSecurityException e) {
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		"Could not use DES cipher - " + e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}
    }
    
   /**
     * Helper routine to decrypt from an InputStream and write the
     * application data straight to an output array with minimal
     * buffer copies. The confounder and the padding are stored
     * separately and not copied into this output array.
     * @param key the DES key to use
     * @param is the InputStream from which the cipher text should be 
     * read
     * @param len the length of the ciphertext data
     * @param dataOutBuf the output buffer where the application data 
     * should be writte
     * @param dataOffset the offser where the application data should 
     * be written.
     * @throws GSSException is an error occurs while decrypting the
     * data
     */
    private void desCbcDecrypt(WrapToken token, byte[] key, 
	InputStream is, int len, byte[] dataOutBuf, int dataOffset)
	throws GSSException, IOException {

	int temp = 0;
	
	Cipher des = getInitializedDes(false, key, ZERO_IV);
	
	WrapTokenInputStream truncatedInputStream = 
	    new WrapTokenInputStream(is, len);
	CipherInputStream cis = new CipherInputStream(truncatedInputStream,
						      des);
	/*
	 * Remove the counfounder first.
	 * CONFOUNDER_SIZE is one DES block ie 8 bytes.
	 */
	temp = cis.read(token.confounder);
	
	len -= temp;
	// temp should be CONFOUNDER_SIZE
	//debug("Got " + temp + " bytes; CONFOUNDER_SIZE is " 
	//     + CONFOUNDER_SIZE + "\n");
	//debug("Confounder is " + getHexBytes(confounder) + "\n");
	
	
	/*
	 * len is a multiple of 8 due to padding.
	 * Decrypt all blocks directly into the output buffer except for
	 * the very last block. Remove the trailing padding bytes from the
	 * very last block and copy that into the output buffer.
	 */
	
	int blockSize = des.getBlockSize();
	int numBlocks = len / blockSize - 1;
	
	// Iterate over all but the last block
	for (int i = 0; i < numBlocks; i++) {
	    //debug("dataOffset is " + dataOffset + "\n");
	    temp = cis.read(dataOutBuf, dataOffset, blockSize);
	    
	    // temp should be blockSize
	    //debug("Got " + temp + " bytes and blockSize is " 
	    //	  + blockSize + "\n");
	    //debug("Bytes are: " 
	    //	  + getHexBytes(dataOutBuf, dataOffset, temp) + "\n");
	    dataOffset += blockSize;
	}
	
	// Now process the last block
	byte[] finalBlock = new byte[blockSize];
	//debug("Will call read on finalBlock" + "\n");
	temp = cis.read(finalBlock);
	// temp should be blockSize
	/*
	  debug("Got " + temp + " bytes and blockSize is " 
	  + blockSize + "\n");
	  debug("Bytes are: " 
	  + getHexBytes(finalBlock, 0, temp) + "\n");
	  debug("Will call doFinal" + "\n");
	*/
	try {
	    des.doFinal();
	} catch (GeneralSecurityException e) {
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		"Could not use DES cipher - " + e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}
	
	/*
	 * There is always at least one padding byte. The padding bytes
	 * are all the value of the number of padding bytes.
	 */
	
	int padSize = finalBlock[blockSize - 1];
	if (padSize < 1  || padSize > 8)
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
				   "Invalid padding on Wrap Token");
	token.padding = WrapToken.pads[padSize];
	blockSize -= padSize;
	
	// Copy this last block into the output buffer
	System.arraycopy(finalBlock, 0, dataOutBuf, dataOffset,
			 blockSize);
    }

    private static byte[] getDesEncryptionKey(byte[] key) 
	throws GSSException {

	/* 
	 * To meet export control requirements, double check that the 
	 * key being used is no longer than 64 bits.  
	 * 
	 * Note that from a protocol point of view, an 
	 * algorithm that is not DES will be rejected before this 
	 * point. Also, a DES key that is not 64 bits will be 
	 * rejected by a good JCE provider.
	 */ 
	if (key.length > 8)
	    throw new GSSException(GSSException.FAILURE, -100,
				   "Invalid DES Key!"); 

	byte[] retVal = new byte[key.length];
	for (int i = 0; i < key.length; i++)
	    retVal[i] = (byte)(key[i] ^ 0xf0);  // RFC 1964, Section 1.2.2
	return retVal;
    }

    // ---- DES3-KD methods
    private void des3KdDecrypt(WrapToken token, byte[] ciphertext, 
	int cStart, int cLen, byte[] plaintext, int pStart) 
	throws GSSException {
	byte[] ptext;
	try {
	    ptext = Des3.decryptRaw(keybytes, KG_USAGE_SEAL, ZERO_IV, 
		ciphertext, cStart, cLen);
	} catch (GeneralSecurityException e) {
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		"Could not use DES3-KD Cipher - " + e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}

	/*
	Krb5Token.debug("\ndes3KdDecrypt in: " + 
	    Krb5Token.getHexBytes(ciphertext, cStart, cLen));
	Krb5Token.debug("\ndes3KdDecrypt plain: " + 
	    Krb5Token.getHexBytes(ptext));
	*/

	// Strip out confounder and padding
	/*
         * There is always at least one padding byte. The padding bytes
	 * are all the value of the number of padding bytes.
	 */
	int padSize = ptext[ptext.length - 1];
	if (padSize < 1  || padSize > 8)
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
		"Invalid padding on Wrap Token");

	token.padding = WrapToken.pads[padSize];
	int len = ptext.length - WrapToken.CONFOUNDER_SIZE - padSize;

	System.arraycopy(ptext, WrapToken.CONFOUNDER_SIZE, 
	    plaintext, pStart, len);

	// Needed to calculate checksum
	System.arraycopy(ptext, 0, token.confounder, 
	    0, WrapToken.CONFOUNDER_SIZE);
    }

    private byte[] des3KdEncrypt(byte[] confounder, byte[] plaintext, 
	int start, int len, byte[] padding) throws GSSException {


    	// [confounder | plaintext | padding]
	byte[] all = new byte[confounder.length + len + padding.length];
	System.arraycopy(confounder, 0, all, 0, confounder.length);
	System.arraycopy(plaintext, start, all, confounder.length, len);
	System.arraycopy(padding, 0, all, confounder.length + len, 
	    padding.length);

	// Krb5Token.debug("\ndes3KdEncrypt:" + Krb5Token.getHexBytes(all));

	// Encrypt
	try {
	    byte[] answer = Des3.encryptRaw(keybytes, KG_USAGE_SEAL, ZERO_IV,
		all, 0, all.length);
	    // Krb5Token.debug("\ndes3KdEncrypt encrypted:" + 
	    //	Krb5Token.getHexBytes(answer));
	    return answer;
	} catch (Exception e) {
	    // GeneralSecurityException, KrbCryptoException
	    GSSException ge = new GSSException(GSSException.FAILURE, -1, 
		"Could not use DES3-KD Cipher - " + e.getMessage());
	    ge.initCause(e);
	    throw ge;
	}
    }

    /**
     * This class provides a truncated inputstream needed by WrapToken. The
     * truncated inputstream is passed to CipherInputStream. It prevents
     * the CipherInputStream from treating the bytes of the following token 
     * as part fo the ciphertext for this token.
     */
    class WrapTokenInputStream extends InputStream {

	private InputStream is;
	private int length;
	private int remaining;

	private int temp;

	public WrapTokenInputStream(InputStream is, int length) {
	    this.is = is;
	    this.length = length;
	    remaining = length;
	}

	public final int read() throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
	        temp = is.read();
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final int read(byte[] b) throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
		temp = Math.min(remaining, b.length);
		temp = is.read(b, 0, temp);
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final int read(byte[] b,
			      int off,
			      int len) throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
		temp = Math.min(remaining, len);
		temp = is.read(b, off, temp);
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final long skip(long n)  throws IOException {
	    if (remaining == 0)
		return 0;
	    else {
		temp = (int) Math.min(remaining, n);
		temp = (int) is.skip(temp);
		remaining -= temp;
		return temp;
	    }
	}

	public final int available() throws IOException {
	    return Math.min(remaining, is.available()); 
	}

	public final void close() throws IOException {
	    remaining = 0;
	}
    }
}
