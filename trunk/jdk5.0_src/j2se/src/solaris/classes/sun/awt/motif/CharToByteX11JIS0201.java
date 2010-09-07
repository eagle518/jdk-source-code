/*
 * @(#)CharToByteX11JIS0201.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;
import  sun.io.*;

import sun.io.CharToByteISO8859_1;

public class CharToByteX11JIS0201 extends CharToByteISO8859_1 {

    public String getCharacterEncoding(){
	return "X11JIS0201";
    }
    
    public boolean canConvert(char ch){
	if ((ch >= 0xff61 && ch <= 0xff9f) 
	    || ch == 0x203e
	    || ch == 0xa5){
	    return true;
	}
	
	return false;
    }

    public int convert(char[] input, int inOff, int inEnd,
		       byte[] output, int outOff, int outEnd)
	throws UnknownCharacterException,
               ConversionBufferFullException
   {
        charOff = inOff;
        byteOff = outOff;

	while (charOff < inEnd && byteOff < outEnd){
	    char ch = input[charOff++]; 
            if (byteOff > outEnd)
	        throw new ConversionBufferFullException();
	    if (ch == 0x203e) {
		output[byteOff++] = (byte)0x7e;
	    } else if (ch == 0xa5) {
		output[byteOff++] = (byte)0x5c;
	    } else if (ch >= 0xff61 && ch <= 0xff9f) {
		output[byteOff++] = (byte)(ch - 0xff61 + 0xa1);
	    } else {
		if (subMode) { 
		    output[byteOff++] = subBytes[0];
		} else {
		    badInputLength = 1;
		    throw new UnknownCharacterException();
		}
	    }
	}
	return byteOff - outOff;
    }
}




