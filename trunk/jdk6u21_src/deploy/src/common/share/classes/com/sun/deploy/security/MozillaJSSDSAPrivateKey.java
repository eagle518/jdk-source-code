/*
 * @(#)MozillaJSSDSAPrivateKey.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.Key;

/**
 * This class encapsulates the handle of the DSA private key in Mozilla JSS APIs.
 *
 * @author Stanley Man-Kit Ho
 */
class MozillaJSSDSAPrivateKey extends MozillaJSSPrivateKey
{
    /**
     * Construct a MozillaJSSDSAPrivateKey object.
     */
    MozillaJSSDSAPrivateKey(Object key, int keyLength)
    {
	super(key, keyLength);
    }
    
    /**
     * Returns the standard algorithm name for this key. For
     * example, "DSA" would indicate that this key is a DSA key.
     * See Appendix A in the <a href=
     * "../../../guide/security/CryptoSpec.html#AppA">
     * Java Cryptography Architecture API Specification &amp; Reference </a>
     * for information about standard algorithm names.
     *
     * @return the name of the algorithm associated with this key.
     */
    public String getAlgorithm()
    {
	return "DSA";
    }
        
    public String toString()
    {
	return "MozillaJSSDSAPrivateKey [JSSKey=" + key + ", key length=" + keyLength + "bits]";
    }
}
