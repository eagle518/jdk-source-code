/*
 * @(#)CharToByteCp834.java	1.2 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM933;

//EBIDIC DBCSONLY Korean

public class CharToByteCp834 extends CharToByteCp933
{
    public CharToByteCp834() {
       super();
       subBytes = new byte[] {(byte)0xfe, (byte)0xfe};
    }

    protected boolean doSBCS() {
        return false;
    }

    protected int encodeHangul(char ch) {
        int theBytes = super.encodeHangul(ch);
        if (theBytes == -1) {
            // Cp834 has 6 additional non-roundtrip char->bytes
            // mappings, see#6379808
	    if (ch == '\u00b7') {
                return 0x4143;
	    } else if (ch == '\u00ad') {
                return 0x4148;
	    } else if (ch == '\u2015') {
                return 0x4149;
	    } else if (ch == '\u223c') {
                return 0x42a1;		  
	    } else if (ch == '\uff5e') {
                return 0x4954;
            } else if (ch == '\u2299') {
                return 0x496f;
            }
	} else if (((theBytes & 0xff00)>>8) == 0) {
	    //SBCS, including 0
            return -1;
        }
	return theBytes;
    }

    public int getMaxBytesPerChar() {
       return 2;
    }

    public String getCharacterEncoding() {
       return "Cp834";
    }
}


