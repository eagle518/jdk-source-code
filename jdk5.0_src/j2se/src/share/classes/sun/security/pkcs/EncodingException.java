/*
 * @(#)EncodingException.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Generic PKCS Encoding exception.
 * 
 * @version 1.3	96/09/15
 * @author Benjamin Renaud 
 */

package sun.security.pkcs;

public class EncodingException extends Exception {

    private static final long serialVersionUID = 4060198374240668325L;

    public EncodingException() {
	super();
    }

    public EncodingException(String s) {
	super(s);
    }
}
