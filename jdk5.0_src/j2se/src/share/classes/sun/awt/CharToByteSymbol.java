/*
 * @(#)CharToByteSymbol.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import sun.io.*;

public class CharToByteSymbol extends CharToByteISO8859_1 {
    
    private static byte[] table_math = {
	(byte)0042, (byte)0000, (byte)0144, (byte)0044,
	(byte)0000, (byte)0306, (byte)0104, (byte)0321,    // 00
	(byte)0316, (byte)0317, (byte)0000, (byte)0000,
	(byte)0000, (byte)0047, (byte)0000, (byte)0120,
	(byte)0000, (byte)0345, (byte)0055, (byte)0000,
	(byte)0000, (byte)0244, (byte)0000, (byte)0052,    // 10
	(byte)0260, (byte)0267, (byte)0326, (byte)0000,
	(byte)0000, (byte)0265, (byte)0245, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0275,
	(byte)0000, (byte)0000, (byte)0000, (byte)0331,    // 20
	(byte)0332, (byte)0307, (byte)0310, (byte)0362,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0134, (byte)0000, (byte)0000, (byte)0000,    // 30
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0176, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0100, (byte)0000, (byte)0000,    // 40
	(byte)0273, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,    // 50
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0271, (byte)0272, (byte)0000, (byte)0000,
	(byte)0243, (byte)0263, (byte)0000, (byte)0000,    // 60
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,    // 70
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0314, (byte)0311,
	(byte)0313, (byte)0000, (byte)0315, (byte)0312,    // 80
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000, 
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0305, (byte)0000, (byte)0304,    // 90
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000, 
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0136, (byte)0000, (byte)0000,    // a0
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,    // b0
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0340, (byte)0327, (byte)0000, (byte)0000,    // c0
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,    // d0
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,    // e0
	(byte)0000, (byte)0000, (byte)0000, (byte)0000,
	(byte)0000, (byte)0000, (byte)0000, (byte)0274,
    };
    
    private static byte[] table_greek = {
	            (byte)0101, (byte)0102, (byte)0107,
	(byte)0104, (byte)0105, (byte)0132, (byte)0110,    // 90
        (byte)0121, (byte)0111, (byte)0113, (byte)0114,
	(byte)0115, (byte)0116, (byte)0130, (byte)0117,
	(byte)0120, (byte)0122, (byte)0000, (byte)0123, 
        (byte)0124, (byte)0125, (byte)0106, (byte)0103,    // a0
	(byte)0131, (byte)0127, (byte)0000, (byte)0000, 
        (byte)0000, (byte)0000, (byte)0000, (byte)0000, 
	(byte)0000, (byte)0141, (byte)0142, (byte)0147, 
        (byte)0144, (byte)0145, (byte)0172, (byte)0150,    // b0
        (byte)0161, (byte)0151, (byte)0153, (byte)0154, 
        (byte)0155, (byte)0156, (byte)0170, (byte)0157, 
        (byte)0160, (byte)0162, (byte)0126, (byte)0163, 
        (byte)0164, (byte)0165, (byte)0146, (byte)0143,    // c0
        (byte)0171, (byte)0167, (byte)0000, (byte)0000, 
        (byte)0000, (byte)0000, (byte)0000, (byte)0000, 
        (byte)0000, (byte)0112, (byte)0241, (byte)0000, 
        (byte)0000, (byte)0152, (byte)0166,                // d0
    };	      
    
    public String toString(){
	return "Symbol";
    }
    
    public boolean canConvert(char ch){
	if (ch >= 0x2200 && ch <= 0x22ef) {
	    if (table_math[ch - 0x2200] != 0x00) {
		return true;
	    }
	} else if (ch >= 0x0391 && ch <= 0x03d6) {
	    if (table_greek[ch - 0x0391] != 0x00) {
		return true;
	    }
	}
	return false;
    }
    
    public int convert(char[] input, int inStart, int inEnd,
		       byte[] output, int outStart, int outEnd) 
	throws UnknownCharacterException,
               ConversionBufferFullException    
    {
	charOff = inStart;
        byteOff = outStart;

	for (int charOff = inStart; charOff < inEnd; charOff++) {
	    char ch = input[charOff];
            if (byteOff > outEnd)
	        throw new ConversionBufferFullException();	    
	    if (ch >= 0x2200 && ch <= 0x22ef){
		output[byteOff++] = table_math[ch - 0x2200];
	    } 
	    else if (ch >= 0x0391 && ch <= 0x03d6) {
		output[byteOff++] = table_greek[ch - 0x0391];
	    }
	    else {
		if (subMode) { 
		    output[byteOff++] = subBytes[0];
		}
		else {
		    badInputLength = 1;
		    throw new UnknownCharacterException();
		}
	    }
	}
	return byteOff - outStart;
    }
}
