/*
 * @(#)CharToByteMS950_HKSCS.java	1.3	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
