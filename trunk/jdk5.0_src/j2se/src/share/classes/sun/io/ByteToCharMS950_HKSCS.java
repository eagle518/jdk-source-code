/*
 * @(#)ByteToCharMS950_HKSCS.java	1.3	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package	sun.io;

public class ByteToCharMS950_HKSCS extends ByteToCharHKSCS {
    ByteToCharMS950 bcMS950 = new ByteToCharMS950();

    public String getCharacterEncoding() {
        return "MS950_HKSCS";
    }

    protected char getUnicode(int byte1, int byte2) {
	char c = super.getUnicode(byte1, byte2);
	return (c != REPLACE_CHAR) ? c : bcMS950.getUnicode(byte1, byte2); 
    }
}
