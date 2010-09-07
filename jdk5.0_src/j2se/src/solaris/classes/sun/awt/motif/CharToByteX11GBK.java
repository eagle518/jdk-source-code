/*
 * @(#)CharToByteX11GBK.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;
import sun.io.CharToByteGBK;

public class CharToByteX11GBK extends CharToByteGBK {

    public String toString(){
	return "X11GBK";
    }

    public boolean canConvert(char ch){
	if (ch < 0x80)
	    return false;
	return super.canConvert(ch);
    }
    
    public int convert(char[] input, int inOff, int inEnd,
                       byte[] output, int outOff, int outEnd) {

	for (int i = inOff; i < inEnd; i++) {
	    int  euc_gbk = getNative(input[i]);
            output[outOff++] = (byte)((euc_gbk & 0xff00) >> 8);
            output[outOff++] = (byte)(euc_gbk & 0xff);
        }
        return (inEnd - inOff) * 2;
    }

}

