/*
 * @(#)ByteToCharMS950_HKSCS.java	1.5	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
