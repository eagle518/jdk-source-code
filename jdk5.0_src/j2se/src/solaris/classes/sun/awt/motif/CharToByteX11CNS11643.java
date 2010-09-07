/*
 * @(#)CharToByteX11CNS11643.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.io.CharToByteEUC_TW;
import sun.io.ConversionBufferFullException;
import sun.io.UnknownCharacterException;


abstract class CharToByteX11CNS11643 extends CharToByteEUC_TW {

    private final int plane;
    private final String encoding;

    CharToByteX11CNS11643(int plane, String encoding) {
        switch (plane) {
        case 1:
            this.plane = 0; // CS1
            break;
        case 2:
        case 3:
            this.plane = plane;
            break;
        default:
            throw new IllegalArgumentException
                ("Only planes 1, 2, and 3 supported");
        }
        this.encoding = encoding;
    }

    public String getCharacterEncoding() {
        return encoding;
    }

    public boolean canConvert(char ch) {
        if (ch <= 0x7F) {
            return false;
        } else {
            int plane = (unicodeToEUC(ch) & 0x00FF0000) >> 16;
            if (plane == 0 && this.plane == 0) {
                return true; // CS1
            } else {
                return (this.plane == (plane - 0xA0)); // CS2
            }
        }
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
                int cns = unicodeToEUC(input[inI++]);
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
