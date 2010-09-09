/*
 * @(#)ByteToCharCp834.java	1.2	10/03/23 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM933;

public class ByteToCharCp834 extends ByteToCharDBCS_ONLY_EBCDIC {
    public String getCharacterEncoding() {
        return "Cp834";
    }

    public ByteToCharCp834() {
        super();
	super.mask1 = 0xFFF0;
	super.mask2 = 0x000F;
	super.shift = 4;
	super.index1 = IBM933.getDecoderIndex1();
	super.index2 = IBM933.getDecoderIndex2();
    }
}

abstract class ByteToCharDBCS_ONLY_EBCDIC extends ByteToCharConverter {
    private boolean savedBytePresent;
    private byte savedByte;

    protected short index1[];
    protected String index2;
    protected int   mask1;
    protected int   mask2;
    protected int   shift;

    public ByteToCharDBCS_ONLY_EBCDIC() {
       super();
       savedBytePresent = false;
    }

    public int flush(char [] output, int outStart, int outEnd)
       throws MalformedInputException
    {
       if (savedBytePresent) {
           reset();
           badInputLength = 0;
           throw new MalformedInputException();
       }
       reset();
       return 0;
    }


    /**
     * Character conversion
     */
    public int convert(byte[] input, int inOff, int inEnd,
                       char[] output, int outOff, int outEnd)
        throws UnknownCharacterException, MalformedInputException,
               ConversionBufferFullException
    {
       int  inputSize;
       char outputChar = '\uFFFD';

       charOff = outOff;
       byteOff = inOff;

       while(byteOff < inEnd) {
          int byte1, byte2;
          int v;

          if (!savedBytePresent) {
              byte1 = input[byteOff] & 0xff;
              inputSize = 1;
          } else {
              byte1 = savedByte;
              savedBytePresent = false;
              inputSize = 0;
          }

          // valid range of 1st bytes
          if (byte1 < 0x40 || byte1 > 0xfe) {
              badInputLength = 1;
              throw new MalformedInputException();
          }

          if (byteOff + inputSize >= inEnd) {
              // We have been split in the middle if a character
              // save the first byte for next time around
              savedByte = (byte)byte1;
              savedBytePresent = true;
              byteOff += inputSize;
              break;
	  }

          byte2 = input[byteOff+inputSize] & 0xff;
          inputSize++;

          // validate the pair of bytes
          if ((byte1 != 0x40 || byte2 != 0x40) &&
	      (byte2 < 0x41 || byte2 > 0xfe)) {
	      badInputLength = 2;
	      throw new MalformedInputException();
	  }

          // Lookup in the two level index
          v = byte1 * 256 + byte2;
	  outputChar = index2.charAt(index1[((v & mask1) >> shift)] 
				     + (v & mask2));

	  if (outputChar == '\uFFFD') {
	      if (subMode)
                  outputChar = subChars[0];
	      else {
		  badInputLength = inputSize;
		  throw new UnknownCharacterException();
	      }
	  }

	  if (charOff >= outEnd)
	      throw new ConversionBufferFullException();

	  output[charOff++] = outputChar;
	  byteOff += inputSize;
       }
       return charOff - outOff;
    }

    /**
     *  Resets the converter.
     */
    public void reset() {
        charOff = byteOff = 0;
        savedBytePresent = false;
    }
}
