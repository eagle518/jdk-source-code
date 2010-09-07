/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DHKey.java	1.4 03/12/19
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.interfaces;

import javax.crypto.spec.DHParameterSpec;

/** 
 * The interface to a Diffie-Hellman key.
 *
 * @author Jan Luehe
 *
 * @version 1.4, 12/19/03
 *
 * @see javax.crypto.spec.DHParameterSpec
 * @see DHPublicKey
 * @see DHPrivateKey
 * @since 1.4
 */
public interface DHKey
{

    /** 
     * Returns the key parameters.
     *
     * @return the key parameters
     */
    public DHParameterSpec getParams();
}
