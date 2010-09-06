/*
 * @(#)ByteToCharEUC.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

/**
* @author Malcolm Ayres
*/
public abstract class ByteToCharEUC extends ByteToCharConverter
{
    private final int G0 = 0;
    private final int G1 = 1;
    private final int SS2 =  0x8E;
    private final int SS3 =  0x8F;

    private int firstByte, state;

    protected String  mappingTableG1;
    protected String  byteToCharTable;


    public ByteToCharEUC() {
        super();
        state = G0;
    }

    /**
      * flush out any residual data and reset the buffer state
      */
    public int flush(char[] output, int outStart, int outEnd)
       throws MalformedInputException
    {
       if (state != G0) {
          reset();
          badInputLength = 0;
          throw new MalformedInputException();
       }

       reset();
       return 0;
    }

    /**
     *  Resets the converter.
     */
    public void reset() {
       state = G0;
       charOff = byteOff = 0;
    }

    /**
     * Character conversion
     */
    public int convert(byte[] input, int inOff, int inEnd,
                       char[] output, int outOff, int outEnd)
        throws UnknownCharacterException, MalformedInputException,
               ConversionBufferFullException
    {

       int       byte1;
       char      outputChar = '\uFFFD';

       byteOff = inOff;
       charOff = outOff;

       while (byteOff < inEnd) {

          byte1 = input[byteOff];
          if (byte1 < 0)
             byte1 += 256;

          switch (state) {
             case G0:
                if (byte1 == SS2 ||                // no general support
                    byte1 == SS3 ) {               //    for g2 or g3
                   badInputLength = 1;
                   throw new MalformedInputException();
                }

                if ( byte1 <= 0x9f )               // < 0x9f has its own table
                   outputChar = byteToCharTable.charAt(byte1);
                else
                   if (byte1 < 0xa1 || byte1 > 0xfe) {  // byte within range?
                      badInputLength = 1;
                      throw new MalformedInputException();
                   } else {                       // G1 set first byte
                      firstByte = byte1;
                      state = G1;
                   }
                break;

             case G1:

                state = G0;
                if ( byte1 < 0xa1 || byte1 > 0xfe) {  // valid G1 set second byte
                   badInputLength = 1;
                   throw new MalformedInputException();
                }

                outputChar = mappingTableG1.charAt(((firstByte - 0xa1) * 94) + byte1 - 0xa1);
                break;

          }

          if (state == G0) {
             if (outputChar == '\uFFFD') {
                if (subMode)
                   outputChar = subChars[0];
                else {
                   badInputLength = 1;
                   throw new UnknownCharacterException();
                }
             }

             if (charOff >= outEnd)
                throw new ConversionBufferFullException();

             output[charOff++] = outputChar;
          }

          byteOff++;

       }

       return charOff - outOff;

   }

}
