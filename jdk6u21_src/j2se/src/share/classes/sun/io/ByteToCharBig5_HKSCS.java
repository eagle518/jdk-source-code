/*
 * @(#)ByteToCharBig5_HKSCS.java	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package	sun.io;

public class ByteToCharBig5_HKSCS extends ByteToCharHKSCS_2001 {
    ByteToCharBig5 bcBig5 = new ByteToCharBig5();

    public String getCharacterEncoding() {
        return "Big5_HKSCS";
    }

    protected char getUnicode(int byte1, int byte2) {
	char c = super.getUnicode(byte1, byte2);
	return (c != REPLACE_CHAR) ? c : bcBig5.getUnicode(byte1, byte2); 
    }
}
