/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DoubleByteDecoder.java	1.5 03/12/19
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;

abstract class DoubleByteDecoder
    extends CharsetDecoder
{

    private short index1[];

    /*
     * 2nd level index, provided by subclass
     * every string has 0x10*(end-start+1) characters.
     */
    private String  index2[];

    protected int start;
    protected int end;

    protected static final char REPLACE_CHAR = '\uFFFD';
    protected char highSurrogate;
    protected char lowSurrogate;

    protected DoubleByteDecoder(Charset cs, short[] index1, String[] index2,
				int start, int end ) {
	super(cs, 0.5f, 1.0f);
	this.index1 = index1;
	this.index2 = index2;
	this.start = start;
	this.end = end;
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

	try {
	    while (sp < sl) {
		int b1, b2;
		b1 = sa[sp];
		int inputSize = 1;
		int outputSize = 1;
		highSurrogate = lowSurrogate = 0;

		char c = decodeSingle(b1);
		if (c == REPLACE_CHAR) {
		    b1 &= 0xff;
		    if (sl - sp < 2)
		        return CoderResult.UNDERFLOW;
		    b2 = sa[sp + 1] & 0xff;
		    c = decodeDouble(b1, b2);
		    if (c == REPLACE_CHAR)
		        return CoderResult.unmappableForLength(1);

		    inputSize = 2;
		    outputSize = (highSurrogate > 0) ? 2: 1;
		}

		if (dl - dp < outputSize)
		    return CoderResult.OVERFLOW;
		if (outputSize == 2) {
		    da[dp++] = highSurrogate;
		    da[dp++] = lowSurrogate;
		} else {
		    da[dp++] = c;
		}

		sp += inputSize;
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
	int outputSize = 0;
	try {
	    while (src.hasRemaining()) {
		int b1 = src.get();
		inputSize = 1;
		outputSize = 1;
		highSurrogate = lowSurrogate = 0;

		char c = decodeSingle(b1);

		if (c == REPLACE_CHAR) {
		    if (src.remaining() < 1)
			return CoderResult.UNDERFLOW;
		    b1 &= 0xff;
		    int b2 = src.get() & 0xff; 
		    inputSize = 2;

		    c = decodeDouble(b1, b2);

		    if (c == REPLACE_CHAR)
			return CoderResult.unmappableForLength(2);

		    outputSize =  (highSurrogate > 0) ? 2: 1;
		}
		if (dst.remaining() < outputSize)
		    return CoderResult.OVERFLOW;
		mark += inputSize;

		if (outputSize == 2) {
		    dst.put(highSurrogate);
		    dst.put(lowSurrogate);
		} else {
		    dst.put(c);
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

    /*
     * Can be changed by subclass
     */
    protected char decodeSingle(int b) {
	if (b >= 0)
	    return (char) b;
	return REPLACE_CHAR;
    }

    protected char decodeDouble(int byte1, int byte2) {
	if (((byte1 < 0) || (byte1 > index1.length))
	    || ((byte2 < start) || (byte2 > end)))
	    return REPLACE_CHAR;

	int n = (index1[byte1] & 0xf) * (end - start + 1) + (byte2 - start);
	return index2[index1[byte1] >> 4].charAt(n);
    }
}
