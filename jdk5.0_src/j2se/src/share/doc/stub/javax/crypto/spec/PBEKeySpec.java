/*
 * @(#)PBEKeySpec.java	1.11 04/06/03
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

package javax.crypto.spec;

import java.security.spec.KeySpec;

/** 
 * A user-chosen password that can be used with password-based encryption
 * (<i>PBE</i>).
 *
 * <p>The password can be viewed as some kind of raw key material, from which
 * the encryption mechanism that uses it derives a cryptographic key.
 *
 * <p>Different PBE mechanisms may consume different bits of each password
 * character. For example, the PBE mechanism defined in
 * <a href="http://www.ietf.org/rfc/rfc2898.txt">
 * PKCS #5</a> looks at only the low order 8 bits of each character, whereas
 * PKCS #12 looks at all 16 bits of each character.
 *
 * <p>You convert the password characters to a PBE key by creating an
 * instance of the appropriate secret-key factory. For example, a secret-key
 * factory for PKCS #5 will construct a PBE key from only the low order 8 bits
 * of each password character, whereas a secret-key factory for PKCS #12 will
 * take all 16 bits of each character.
 *
 * <p>Also note that this class stores passwords as char arrays instead of
 * <code>String</code> objects (which would seem more logical), because the 
 * String class is immutable and there is no way to overwrite its
 * internal value when the password stored in it is no longer needed. Hence,
 * this class requests the password as a char array, so it can be overwritten
 * when done.
 *
 * @author Jan Luehe
 * @author Valerie Peng
 *
 * @version 1.17, 06/03/04
 *
 * @see javax.crypto.SecretKeyFactory
 * @see PBEParameterSpec
 * @since 1.4
 */
public class PBEKeySpec implements KeySpec
{

    /** 
     * Constructor that takes a password. An empty char[] is used if
     * null is specified.
     *
     * <p> Note: <code>password</code> is cloned before it is stored in
     * the new <code>PBEKeySpec</code> object.
     *
     * @param password the password.
     */
    public PBEKeySpec(char[] password) { }

    /** 
     * Constructor that takes a password, salt, iteration count, and
     * to-be-derived key length for generating PBEKey of variable-key-size
     * PBE ciphers.  An empty char[] is used if null is specified for 
     * <code>password</code>.
     *
     * <p> Note: the <code>password</code> and <code>salt</code> 
     * are cloned before they are stored in
     * the new <code>PBEKeySpec</code> object.
     *
     * @param password the password.
     * @param salt the salt.
     * @param iterationCount the iteration count.
     * @param keyLength the to-be-derived key length.
     * @exception NullPointerException if <code>salt</code> is null.
     * @exception IllegalArgumentException if <code>salt</code> is empty,
     * i.e. 0-length, <code>iterationCount</code> or
     * <code>keyLength</code> is not positive.
     */
    public PBEKeySpec(char[] password, byte[] salt, int iterationCount, int
        keyLength)
    { }

    /** 
     * Constructor that takes a password, salt, iteration count for
     * generating PBEKey of fixed-key-size PBE ciphers. An empty 
     * char[] is used if null is specified for <code>password</code>.
     *
     * <p> Note: the <code>password</code> and <code>salt</code>
     * are cloned before they are stored in the new 
     * <code>PBEKeySpec</code> object.
     *
     * @param password the password.
     * @param salt the salt.
     * @param iterationCount the iteration count.
     * @exception NullPointerException if <code>salt</code> is null.
     * @exception IllegalArgumentException if <code>salt</code> is empty,
     * i.e. 0-length, or <code>iterationCount</code> is not positive.
     */
    public PBEKeySpec(char[] password, byte[] salt, int iterationCount) { }

    /** 
     * Clears the internal copy of the password.
     *
     */
    public final void clearPassword() { }

    /** 
     * Returns a copy of the password.
     *
     * <p> Note: this method returns a copy of the password. It is
     * the caller's responsibility to zero out the password information after
     * it is no longer needed.
     *
     * @exception IllegalStateException if password has been cleared by 
     * calling <code>clearPassword</code> method.
     * @return the password.
     */
    public final char[] getPassword() {
        return null;
    }

    /** 
     * Returns a copy of the salt or null if not specified.
     *
     * <p> Note: this method should return a copy of the salt. It is
     * the caller's responsibility to zero out the salt information after
     * it is no longer needed.
     *
     * @return the salt.
     */
    public final byte[] getSalt() {
        return null;
    }

    /** 
     * Returns the iteration count or 0 if not specified.
     *
     * @return the iteration count.
     */
    public final int getIterationCount() {
        return 0;
    }

    /** 
     * Returns the to-be-derived key length or 0 if not specified.
     *
     * <p> Note: this is used to indicate the preference on key length
     * for variable-key-size ciphers. The actual key size depends on
     * each provider's implementation.
     *
     * @return the to-be-derived key length.
     */
    public final int getKeyLength() {
        return 0;
    }
}
