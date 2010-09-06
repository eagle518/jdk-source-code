/*
 * @(#)UnicodeDecoder.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;


abstract class UnicodeDecoder extends CharsetDecoder {

    protected static final char BYTE_ORDER_MARK = (char) 0xfeff;
    protected static final char REVERSED_MARK = (char) 0xfffe;

    protected static final int NONE = 0;
    protected static final int BIG = 1;
    protected static final int LITTLE = 2;

    private final int expectedByteOrder; 
    private int currentByteOrder;

    public UnicodeDecoder(Charset cs, int bo) {
	super(cs, 0.5f, 1.0f); 
	expectedByteOrder = currentByteOrder = bo;
    }

    private char decode(int b1, int b2) {
	if (currentByteOrder == BIG)
	    return (char)((b1 << 8) | b2);
	else
	    return (char)((b2 << 8) | b1);
    }

    protected CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
	int mark = src.position();

	try {
	    while (src.remaining() > 1) {
		int b1 = src.get() & 0xff;
		int b2 = src.get() & 0xff;

		// Byte Order Mark interpretation
		if (currentByteOrder == NONE) {
		    char c = (char)((b1 << 8) | b2);
		    if (c == BYTE_ORDER_MARK) {
			currentByteOrder = BIG;
			mark += 2;
			continue;
		    } else if (c == REVERSED_MARK) {
			currentByteOrder = LITTLE;
			mark += 2;
			continue;
		    } else {
			currentByteOrder = BIG;
			// FALL THROUGH to process b1, b2 normally
		    }
		}

		char c = decode(b1, b2);

		if (c == REVERSED_MARK) {
		    // A reversed BOM cannot occur within middle of stream
		    return CoderResult.malformedForLength(2);
		}

		// Surrogates
		if (Surrogate.is(c)) {
		    if (Surrogate.isHigh(c)) {
			if (src.remaining() < 2)
			    return CoderResult.UNDERFLOW;
			char c2 = decode(src.get() & 0xff, src.get() & 0xff);
			if (!Surrogate.isLow(c2))
			    return CoderResult.malformedForLength(4);
			if (dst.remaining() < 2)
			    return CoderResult.UNDERFLOW;
			mark += 4;
			dst.put(c);
			dst.put(c2);
			continue;
		    }
		    // Unpaired low surrogate
		    return CoderResult.malformedForLength(2);
		}

		if (!dst.hasRemaining())
		    return CoderResult.UNDERFLOW;
		mark += 2;
		dst.put(c);

	    }
	    return CoderResult.UNDERFLOW;

	} finally {
	    src.position(mark);
	}
    }

    protected void implReset() {
	currentByteOrder = expectedByteOrder;
    }

}
