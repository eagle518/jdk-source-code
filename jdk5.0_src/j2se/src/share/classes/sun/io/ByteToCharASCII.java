/*
 * @(#)ByteToCharASCII.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * A algorithmic conversion from ASCII to Unicode
 *
 * @author Limin Shi
 */
public class ByteToCharASCII extends ByteToCharConverter {

    // Return the character set id
    public String getCharacterEncoding()
    {
        return "ASCII";
    }

    public int flush(char[] output, int outStart, int outEnd) {
	// This converter will not buffer any data.
	byteOff = charOff = 0;
	return 0;
    }

    /**
     * Algorithmic character conversion
     */
    public int convert(byte[] input, int inOff, int inEnd,
		       char[] output, int outOff, int outEnd)
        throws ConversionBufferFullException, UnknownCharacterException
    {
        byte    inputByte;

        charOff = outOff;
        byteOff = inOff;

        // Loop until we hit the end of the input
        while(byteOff < inEnd)
        {
	    // If we don't have room for the output, throw an exception
	    if (charOff >= outEnd)
		throw new ConversionBufferFullException();

            // Convert the input byte
            inputByte = input[byteOff++];

            if (inputByte >= 0)
            	output[charOff++] = (char)inputByte;
            else {
                if (subMode)
                    output[charOff++] = '\uFFFD';	// Replace Char
                else {
                    badInputLength = 1;
                    throw new UnknownCharacterException();
                }
            }
        }

    	// Return the length written to the output buffer
	return charOff-outOff;
    }

    /*
     *   Reset after finding bad input
     */
    public void reset() {
	byteOff = charOff = 0;
    }

}
