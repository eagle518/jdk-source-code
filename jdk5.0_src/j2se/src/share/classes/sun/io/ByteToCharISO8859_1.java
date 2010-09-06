/*
 * @(#)ByteToCharISO8859_1.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

/**
* A algorithmic conversion from ISO 8859-1 to Unicode
*
* @author Lloyd Honomichl
* @author Asmus Freytag
*/
public class ByteToCharISO8859_1 extends ByteToCharConverter {

    // Return the character set id
    public String getCharacterEncoding()
    {
        return "ISO8859_1";
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

        throws ConversionBufferFullException
    {
        int bound = inOff + (outEnd - outOff);
        if (bound >= inEnd) {
             bound = inEnd;
        }
	int bytesWritten = inEnd - inOff;


        // Loop until we hit the end of the input
	try {
	    while(inOff < bound) {
		output[outOff++] = (char) (0xff & input[inOff++]);
	    }
	} finally {
	    charOff = outOff;
	    byteOff = inOff;
	}

	// If we don't have room for the output, throw an exception
	if (bound < inEnd)
	    throw new ConversionBufferFullException();

    	// Return the length written to the output buffer
	return bytesWritten;
    }

    /*
        Reset after finding bad input
    */
    public void reset() {
	byteOff = charOff = 0;
    }

}
