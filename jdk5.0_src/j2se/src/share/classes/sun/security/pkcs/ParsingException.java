/*
 * @(#)ParsingException.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Generic PKCS Parsing exception. 
 * 
 * @version 1.15	12/19/03
 * @author Benjamin Renaud 
 */

package sun.security.pkcs;

import java.io.IOException;

public class ParsingException extends IOException {

    private static final long serialVersionUID = -6316569918966181883L;

    public ParsingException() {
	super();
    }

    public ParsingException(String s) {
	super(s);
    }
}
