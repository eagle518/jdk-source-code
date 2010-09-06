/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SimpleEUCDecoder.java	1.2	03/12/19
 */

/**
 * Simple EUC-like decoder used by IBM01383 and IBM970
 * supports G1 - no support for G2 or G3
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;

abstract class SimpleEUCDecoder
    extends CharsetDecoder
{

    private final int G0 = 0;
    private final int G1 = 1;
    private final int SS2 =  0x8E;
    private final int SS3 =  0x8F;

    private int firstByte, state;

    protected static String  mappingTableG1;
    protected static String  byteToCharTable;

    protected SimpleEUCDecoder(Charset cs) {
	super(cs, 0.5f, 1.0f);
	state = G0;
    }

    private CoderResult decodeArrayLoop(ByteBuffer src, CharBuffer dst) {
	byte[] sa = src.array();
	int sp = src.arrayOffset() + src.position();
	int sl = src.arrayOffset() + src.limit();
	assert (sp <= sl);
	sp = (sp <= sl ? sp : sl);
	char[] da = dst.array();
	int dp = dst.arrayOffset() + dst.position();
	int dl = dst.arrayOffset() + dst.limit();
	assert (dp <= dl);
	dp = (dp <= dl ? dp : dl);

	int inputSize = 1;

	try {
	    while (sp < sl) {
                int byte1;
		inputSize = 1;
                char outputChar = '\uFFFD';

		byte1 = sa[sp];
		if (byte1 < 0)
		   byte1 += 256;

		switch (state) {
		    case G0:
		    if (byte1 == SS2 ||
			byte1 == SS3 ) {
			// No support provided for G2/G3 at this time.
			CoderResult.malformedForLength(1);
		    }

		    if ( byte1 <= 0x9f )   // < 0x9f has its own table
		       outputChar = byteToCharTable.charAt(byte1);
		    else if (byte1 < 0xa1 || byte1 > 0xfe) {  // valid range?
			CoderResult.malformedForLength(1);
		    } else { // G1 set first byte
			firstByte = byte1;
			state = G1;
		    }
		    break;
		    case G1:
			state = G0;
			if ( byte1 < 0xa1 || byte1 > 0xfe) { 
			    CoderResult.malformedForLength(2);
			}
			outputChar = mappingTableG1.charAt(((firstByte - 0xa1) * 94) + byte1 - 0xa1);
			inputSize = 2;
			break;
		    } // end switch

		if (state == G0) {
		    if  (outputChar == '\uFFFD') {
			return CoderResult.unmappableForLength(inputSize);
		    }

		    if (dl - dp < 1)
			return CoderResult.OVERFLOW;
		    da[dp++] = outputChar;
		}
		sp++;
	    }
	    return CoderResult.UNDERFLOW;
	} finally {
	    src.position(sp - src.arrayOffset());
	    dst.position(dp - dst.arrayOffset());
	}
    }

    private CoderResult decodeBufferLoop(ByteBuffer src, CharBuffer dst) {
	int mark = src.position();
	int inputSize = 0;
	try {
	    while (src.hasRemaining()) {
                char outputChar = '\uFFFD';

		byte byte1 = src.get();
		if (byte1 < 0)
		   byte1 += 256;
		inputSize = 1;

		switch (state) {
		    case G0:
		    if (byte1 == SS2 ||
			byte1 == SS3 ) {
			// No support provided for G2/G3 at this time.
			CoderResult.malformedForLength(inputSize);
		    }

		    if ( byte1 <= 0x9f )   // < 0x9f has its own table
		       outputChar = byteToCharTable.charAt(byte1);
		    else if (byte1 < 0xa1 || byte1 > 0xfe) {  // valid range?
			CoderResult.malformedForLength(1);
		    } else { // G1 set first byte
			firstByte = byte1;
			state = G1;
		    }
		    break;
		    case G1:
			state = G0;
			if ( byte1 < 0xa1 || byte1 > 0xfe) { 
			    CoderResult.malformedForLength(2);
			}
			outputChar = mappingTableG1.charAt(((firstByte - 0xa1) * 94) + byte1 - 0xa1);
			inputSize = 2;
			break;
		}  // end switch

		if (state == G0) {
		    if (outputChar == '\uFFFD') {
			return CoderResult.unmappableForLength(inputSize);
		    }
		    if (!dst.hasRemaining())
			return CoderResult.OVERFLOW;
		    mark++;
		    dst.put(outputChar);
		}
	    }
	    return CoderResult.UNDERFLOW;
	} finally {
	    src.position(mark);
	}
    }

    protected CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
	if (src.hasArray() && dst.hasArray())
	    return decodeArrayLoop(src, dst);
	else
	    return decodeBufferLoop(src, dst);
    }
}
