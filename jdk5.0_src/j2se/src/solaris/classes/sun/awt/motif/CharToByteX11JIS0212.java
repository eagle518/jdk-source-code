/*
 * @(#)CharToByteX11JIS0212.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;
import sun.io.CharToByteJIS0212;

public class CharToByteX11JIS0212 extends CharToByteJIS0212 {

    public String getCharacterEncoding() {
	return "X11JIS0212";
    }

    public boolean canConvert(char ch){
	if (getNative(ch) != 0){
	    return true;
	}
	return false;
    }

    public int convert(char[] input, int inOff, int inEnd,
		       byte[] output, int outOff, int outEnd){

	for (int i = inOff; i < inEnd; i++) {
	    char ch = input[i];
	    
	    int jishex = getNative(ch);

	    //	    output[outOff++] = (byte)0x8f;
	    output[outOff++] = (byte)(jishex >> 8);
	    output[outOff++] = (byte)(jishex & 0xff);
	}

	return (inEnd - inOff) * 2;
    }
}

