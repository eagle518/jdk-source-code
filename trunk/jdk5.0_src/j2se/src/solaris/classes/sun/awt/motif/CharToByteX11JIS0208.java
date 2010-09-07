/*
 * @(#)CharToByteX11JIS0208.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;
import sun.io.CharToByteJIS0208;

public class CharToByteX11JIS0208 extends CharToByteJIS0208 {

    public String getCharacterEncoding() {
	return "X11JIS0208";
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

	    output[outOff++] = (byte)(jishex >> 8);
	    output[outOff++] = (byte)(jishex & 0xff);
	}

	return (inEnd - inOff) * 2;
    }

}

