/*
 * @(#)ByteToCharX11KSC5601.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.io.ByteToCharEUC_KR;
import sun.io.ConversionBufferFullException;
import sun.io.MalformedInputException;
import sun.io.UnknownCharacterException;


public class ByteToCharX11KSC5601 extends ByteToCharEUC_KR {

    private byte leftOverByte;
    private boolean leftOver;

    public String getCharacterEncoding() {
        return "X11KSC5601";
    }

    public int convert(byte[] input, int inOff, int inEnd,
                       char[] output, int outOff, int outEnd)
        throws ConversionBufferFullException, UnknownCharacterException
    {
        byteOff = inOff;
        charOff = outOff;

        if (inOff >= inEnd) {
            return 0;
        }

        byte b1, b2;
        int bc, inI = inOff, outI = outOff;

        if (leftOver) {
            b1 = leftOverByte;
            leftOver = false;
        } else {
            b1 = input[inI++];
        }
        bc = 1;

        while (inI < inEnd) {
            b2 = input[inI++];
            bc = 2;

            if (outI >= outEnd) {
                throw new ConversionBufferFullException();
            }

            char c = getUnicode(b1 & 0xFF | 0x80, b2 & 0xFF | 0x80);
            if (c == REPLACE_CHAR) {
                if (subMode) {
                    c = (subChars.length > 0) ? subChars[0] : '\0';
                } else {
                    throw new UnknownCharacterException();
                }
            }

            output[outI++] = c;
            byteOff = inI;
            charOff = outI;

            if (inI < inEnd) {
                b1 = input[inI++];
                bc = 1;
            }
        }

        if (bc == 1) {
            leftOverByte = b1;
            byteOff = inI;
            leftOver = true;
        }

        return outI - outOff;
    }

    public void reset() {
        leftOver = false;
        byteOff = charOff = 0;
    }

    public int flush(char buf[], int off, int len)
        throws MalformedInputException
    {
        boolean invalid = leftOver;
        reset();
        if (invalid) {
            throw new MalformedInputException();
        }
        return 0;
    }
}
