/*
 * @(#)CharToByteX11Dingbats.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import sun.io.CharToByteISO8859_1;
import sun.io.UnknownCharacterException;

public class CharToByteX11Dingbats extends CharToByteISO8859_1 {

    private static byte[] table =
	{(byte)0xa1, (byte)0xa2, (byte)0xa3, (byte)0xa4,
	 (byte)0xa5, (byte)0xa6, (byte)0xa7,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0xb6, (byte)0xb7,
	 (byte)0xb8, (byte)0xb9, (byte)0xba, (byte)0xbb,
	 (byte)0xbc, (byte)0xbd, (byte)0xbe, (byte)0xbf,
	 (byte)0xc0, (byte)0xc1, (byte)0xc2, (byte)0xc3,
	 (byte)0xc4, (byte)0xc5, (byte)0xc6, (byte)0xc7,
	 (byte)0xc8, (byte)0xc9, (byte)0xca, (byte)0xcb,
	 (byte)0xcc, (byte)0xcd, (byte)0xce, (byte)0xcf, 
	 (byte)0xd0, (byte)0xd1, (byte)0xd2, (byte)0xd3,
	 (byte)0xd4, (byte)0x00, (byte)0x00, (byte)0x00, 
	 (byte)0xd8, (byte)0xd9, (byte)0xda, (byte)0xdb,
	 (byte)0xdc, (byte)0xdd, (byte)0xde, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
	 (byte)0x00, (byte)0x00, (byte)0x00};


    public String toString() {
	return "X11Dingbats";
    }

    public boolean canConvert(char ch) {
        if (ch >= 0x2701 && ch <= 0x275e) {	// direct map
             return true;
	}
        if (ch >= 0x2761 && ch <= 0x27be) {
             return (table[ch - 0x2761] != 0x00);
	}
        return false;
    }


    public int convert(char[] input, int inOff, int inEnd,
		       byte[] output, int outOff, int outEnd) 
    throws UnknownCharacterException {

	int len = inEnd - inOff;
	for (int i = 0; i < len; i++) {
	    char ch = input[inOff + i];
	    if (!canConvert(ch)) {
                if (subMode) {
                    output[outOff + i] = subBytes[0];
                    continue;                    
                } else {                
                    /* this is 1-1 */
		    byteOff = outOff + i;
		    charOff = inOff + i;
                    badInputLength = 1;
		    throw new UnknownCharacterException("can't do: " + (int)ch);
		}
	    }
	    if (ch >= 0x2761){
		    output[outOff + i] = table[ch - 0x2761]; // table lookup
	    } else {
		    output[outOff + i] = (byte)(ch + 0x20 - 0x2700); // direct map
	    }
	}
	return len;
    }
}

