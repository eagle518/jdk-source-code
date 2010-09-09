/*
 * @(#)CharToByteMS950_HKSCS.java	1.5	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

public class CharToByteMS950_HKSCS extends CharToByteHKSCS {
    CharToByteMS950 cbMS950 = new CharToByteMS950();

    public String getCharacterEncoding() {
        return "MS950_HKSCS";
    }

    protected int getNative(char ch) {
	int r = super.getNative(ch);
	return (r != 0) ? r : cbMS950.getNative(ch);
    }
}
