/*
 * @(#)CharToByteX11GB2312.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.io.CharToByteEUC_CN;
import sun.io.ConversionBufferFullException;
import sun.io.UnknownCharacterException;


public class CharToByteX11GB2312 extends CharToByteEUC_CN {

    public String getCharacterEncoding() {
        return "X11GB2312";
    }

    public boolean canConvert(char ch){
        return (ch > 0x7F) ? super.canConvert(ch) : false;
    }

    public int convert(char[] input, int inOff, int inEnd,
                       byte[] output, int outOff, int outEnd)
        throws ConversionBufferFullException, UnknownCharacterException
    {
        charOff = inOff;
        byteOff = outOff;

        if (inOff >= inEnd) {
            return 0;
        }

        int inI = inOff, outI = outOff, outTop = outEnd - 2;

        while (inI < inEnd) {
            if (outI > outTop) {
                charOff = inI;
                byteOff = outI;
                throw new ConversionBufferFullException();
            }
            if (canConvert(input[inI])) {
                int cns = getNative(input[inI++]);
                output[outI++] = (byte)((cns >> 8) & 0x7f);
                output[outI++] = (byte)(cns & 0x7f);
            } else if (subMode) {
                inI++;
                output[outI++] = (subBytes.length > 0) ? subBytes[0] : 0;
                output[outI++] = (subBytes.length > 1) ? subBytes[1] : 0;
            } else {
                charOff = inI;
                byteOff = outI;
                throw new UnknownCharacterException();
            }
        }

        charOff = inI;
        byteOff = outI;

        return outI - outOff;
    }

    public void reset() {
        byteOff = charOff = 0;
    }

    public int flush(byte in[], int inOff, int inEnd) {
        reset();
        return 0;
    }

    public int getMaxBytesPerChar() {
        return 2;
    }
}
