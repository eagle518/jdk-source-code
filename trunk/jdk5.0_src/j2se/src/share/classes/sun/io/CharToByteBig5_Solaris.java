/*
 * @(#)CharToByteBig5_Solaris.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

/**
 */

public class CharToByteBig5_Solaris extends CharToByteBig5 {

    public String getCharacterEncoding() {
        return "Big5_Solaris";
    }

    protected int getNative(char ch) {
	int nativeVal;

	if ((nativeVal = super.getNative(ch)) != 0) {
	    return nativeVal;
	}
	
	switch (ch) {
	    case 0x7881:
		nativeVal = 0xF9D6;
		break;
	    case 0x92B9:
		nativeVal = 0xF9D7;
		break;
	    case 0x88CF:
		nativeVal = 0xF9D8;
		break;
	    case 0x58BB:
		nativeVal = 0xF9D9;
		break;
	    case 0x6052:
		nativeVal = 0xF9DA;
		break;
	    case 0x7CA7:
		nativeVal = 0xF9DB;
		break;
	    case 0x5AFA:
		nativeVal = 0xF9DC;
		break;
	    }
	return nativeVal;
    }
}
