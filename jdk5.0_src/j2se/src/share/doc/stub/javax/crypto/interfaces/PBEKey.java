/*
 * @(#)PBEKey.java	1.6 04/01/14
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

package javax.crypto.interfaces;

import java.math.BigInteger;

/** 
 * The interface to a PBE key.
 *
 * @author Valerie Peng 
 *
 * @version 1.7, 01/14/04
 *
 * @see javax.crypto.spec.PBEKeySpec
 * @see javax.crypto.SecretKey
 * @since 1.4
 */
public interface PBEKey extends javax.crypto.SecretKey
{
    /** 
     * The class fingerprint that is set to indicate serialization
     * compatibility since J2SE 1.4. 
     */
    public static final long serialVersionUID = -1430015993304333921L;

    /** 
     * Returns the password.
     *
     * <p> Note: this method should return a copy of the password. It is
     * the caller's responsibility to zero out the password information after
     * it is no longer needed.
     *
     * @return the password.
     */
    public char[] getPassword();

    /** 
     * Returns the salt or null if not specified.
     *
     * <p> Note: this method should return a copy of the salt. It is
     * the caller's responsibility to zero out the salt information after
     * it is no longer needed.
     *
     * @return the salt.
     */
    public byte[] getSalt();

    /** 
     * Returns the iteration count or 0 if not specified.
     *
     * @return the iteration count.
     */
    public int getIterationCount();
}
