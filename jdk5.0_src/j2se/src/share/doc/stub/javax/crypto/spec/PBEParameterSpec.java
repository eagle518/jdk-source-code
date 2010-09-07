/*
 * @(#)PBEParameterSpec.java	1.7 04/06/03
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

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;

/** 
 * This class specifies the set of parameters used with password-based
 * encryption (PBE), as defined in the
 * <a href="http://www.ietf.org/rfc/rfc2898.txt">PKCS #5</a>
 * standard.
 * 
 * @author Jan Luehe
 *
 * @version 1.16, 06/03/04
 * @since 1.4
 */
public class PBEParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Constructs a parameter set for password-based encryption as defined in
     * the PKCS #5 standard.
     *
     * @param salt the salt. The contents of <code>salt</code> are copied 
     * to protect against subsequent modification.
     * @param iterationCount the iteration count.
     * @exception NullPointerException if <code>salt</code> is null.
     */
    public PBEParameterSpec(byte[] salt, int iterationCount) { }

    /** 
     * Returns the salt.
     *
     * @return the salt. Returns a new array
     * each time this method is called.
     */
    public byte[] getSalt() {
        return null;
    }

    /** 
     * Returns the iteration count.
     *
     * @return the iteration count
     */
    public int getIterationCount() {
        return 0;
    }
}
