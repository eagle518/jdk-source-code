/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ExemptionMechanismException.java	1.4 03/12/19
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.security.GeneralSecurityException;

/** 
 * This is the generic ExemptionMechanism exception.
 *
 * @version 1.4, 12/19/03
 * @since 1.4
 */
public class ExemptionMechanismException extends GeneralSecurityException
{

    /** 
     * Constructs a ExemptionMechanismException with no detailed message.
     * (A detailed message is a String that describes this particular
     * exception.)
     */
    public ExemptionMechanismException() { }

    /** 
     * Constructs a ExemptionMechanismException with the specified
     * detailed message. (A detailed message is a String that describes
     * this particular exception.)
     *
     * @param msg the detailed message.
     */
    public ExemptionMechanismException(String msg) { }
}
