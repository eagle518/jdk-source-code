/*
 * @(#)DHKey.java	1.8 10/03/23
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

package javax.crypto.interfaces;

import javax.crypto.spec.DHParameterSpec;

/** 
 * The interface to a Diffie-Hellman key.
 *
 * @author Jan Luehe
 *
 * @version 1.7, 01/06/04
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
