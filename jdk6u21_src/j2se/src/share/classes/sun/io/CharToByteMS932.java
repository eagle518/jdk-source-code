/*
 * @(#)CharToByteMS932.java	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * Tables and data to convert Unicode to MS932
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteMS932 extends CharToByteMS932DB {
    CharToByteJIS0201 cbJIS0201 = new CharToByteJIS0201();

    public String getCharacterEncoding() {
        return "MS932";
    }

    protected int convSingleByte(char inputChar, byte[] outputByte) {
	byte b;

	// \u0000 - \u007F map straight through
	if ((inputChar &0xFF80) == 0) {
	    outputByte[0] = (byte)inputChar;
	    return 1;
	}

	if ((b = cbJIS0201.getNative(inputChar)) == 0)
	    return 0;

	outputByte[0] = b;
	return 1;
    }
}

