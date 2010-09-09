/*
 * @(#)StandaloneByteArrayAccess.java	1.4 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * This is a pure subset of package-private class
 * sun.security.provider.ByteArrayAccess. The subset consists of only the simple
 * shift and boolean operations needed for the one current client of this 
 * class (sun.jkernel.StandaloneSHA) and omits optimization code and comments 
 * not relevant to the subset.  No semantic changes have been made.  
 * A few long lines were broken to conform to JDK coding style.
 * Pete Soper, August, 2007.
 */

package sun.jkernel;

/**
 * Methods for converting between byte[] and int[]/long[].
 *
 * @since   1.6
 * @version 1.1, 05/26/06
 * @author  Andreas Sterbenz
 */
final class StandaloneByteArrayAccess {
    
    private StandaloneByteArrayAccess() {
        // empty
    }
    
    /**
     * byte[] to int[] conversion, little endian byte order.
     */
    static void b2iLittle(byte[] in, int inOfs, int[] out, int outOfs, 
        int len) {
        len += inOfs;
        while (inOfs < len) {
            out[outOfs++] = ((in[inOfs    ] & 0xff)      )
                          | ((in[inOfs + 1] & 0xff) <<  8)
                          | ((in[inOfs + 2] & 0xff) << 16)
                          | ((in[inOfs + 3]       ) << 24);
            inOfs += 4;
        }
    }
    
    /**
     * int[] to byte[] conversion, little endian byte order.
     */
    static void i2bLittle(int[] in, int inOfs, byte[] out, int outOfs, 
        int len) {
        len += outOfs;
        while (outOfs < len) {
            int i = in[inOfs++];
            out[outOfs++] = (byte)(i      );
            out[outOfs++] = (byte)(i >>  8);
            out[outOfs++] = (byte)(i >> 16);
            out[outOfs++] = (byte)(i >> 24);
        }
    }

    /**
     * byte[] to int[] conversion, big endian byte order.
     */
    static void b2iBig(byte[] in, int inOfs, int[] out, int outOfs, int len) {
        len += inOfs;
        while (inOfs < len) {
            out[outOfs++] = ((in[inOfs + 3] & 0xff)      )
                          | ((in[inOfs + 2] & 0xff) <<  8)
                          | ((in[inOfs + 1] & 0xff) << 16)
                          | ((in[inOfs    ]       ) << 24);
            inOfs += 4;
        }
    }
    
    /**
     * int[] to byte[] conversion, big endian byte order.
     */
    static void i2bBig(int[] in, int inOfs, byte[] out, int outOfs, int len) {
        len += outOfs;
        while (outOfs < len) {
            int i = in[inOfs++];
            out[outOfs++] = (byte)(i >> 24);
            out[outOfs++] = (byte)(i >> 16);
            out[outOfs++] = (byte)(i >>  8);
            out[outOfs++] = (byte)(i      );
        }
    }

    // Store one 32-bit value into out[outOfs..outOfs+3] in big endian order.
    static void i2bBig4(int val, byte[] out, int outOfs) {
        out[outOfs    ] = (byte)(val >> 24);
        out[outOfs + 1] = (byte)(val >> 16);
        out[outOfs + 2] = (byte)(val >>  8);
        out[outOfs + 3] = (byte)(val      );
    }

    /**
     * byte[] to long[] conversion, big endian byte order.
     */
    static void b2lBig(byte[] in, int inOfs, long[] out, int outOfs, int len) {
        len += inOfs;
        while (inOfs < len) {
            int i1 = ((in[inOfs + 3] & 0xff)      )
                   | ((in[inOfs + 2] & 0xff) <<  8)
                   | ((in[inOfs + 1] & 0xff) << 16)
                   | ((in[inOfs    ]       ) << 24);
            inOfs += 4;
            int i2 = ((in[inOfs + 3] & 0xff)      )
                   | ((in[inOfs + 2] & 0xff) <<  8)
                   | ((in[inOfs + 1] & 0xff) << 16)
                   | ((in[inOfs    ]       ) << 24);
            out[outOfs++] = ((long)i1 << 32) | (i2 & 0xffffffffL);
            inOfs += 4;
        }
    }

    /**
     * long[] to byte[] conversion
     */
    static void l2bBig(long[] in, int inOfs, byte[] out, int outOfs, int len) {
        len += outOfs;
        while (outOfs < len) {
            long i = in[inOfs++];
            out[outOfs++] = (byte)(i >> 56);
            out[outOfs++] = (byte)(i >> 48);
            out[outOfs++] = (byte)(i >> 40);
            out[outOfs++] = (byte)(i >> 32);
            out[outOfs++] = (byte)(i >> 24);
            out[outOfs++] = (byte)(i >> 16);
            out[outOfs++] = (byte)(i >>  8);
            out[outOfs++] = (byte)(i      );
        }
    }
    
}
