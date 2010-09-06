/*
 * @(#)ByteToCharBig5_Solaris.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/*
 *
 *
 *
 *
 */
public class ByteToCharBig5_Solaris extends ByteToCharBig5 {
    public ByteToCharBig5_Solaris() {}

    public String getCharacterEncoding() {
        return "Big5_Solaris";
    }

    protected char getUnicode(int byte1, int byte2) {
	//
	char c = super.getUnicode(byte1, byte2);
	if (c == REPLACE_CHAR) {
	    if (byte1 == 0xf9) {
		switch (byte2) {
		    case 0xD6:
			c = (char)0x7881;
			break;
		    case 0xD7:
			c = (char)0x92B9;
			break;
		    case 0xD8:
			c = (char)0x88CF;
			break;
		    case 0xD9:
			c = (char)0x58BB;
			break;
		    case 0xDA:
			c = (char)0x6052;
			break;
		    case 0xDB:
			c = (char)0x7CA7;
			break;
		    case 0xDC:
			c = (char)0x5AFA;
			break;
		}
	    }
	}
	return c;
    }
}
