/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SingleByteDecoder.java	1.13 03/12/19
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;
import java.nio.charset.UnmappableCharacterException;


public abstract class SingleByteDecoder
    extends CharsetDecoder
{

    private final String byteToCharTable;

    protected SingleByteDecoder(Charset cs, String byteToCharTable) {
	super(cs, 1.0f, 1.0f);
	this.byteToCharTable = byteToCharTable;
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
		int b = sa[sp];

		char c = decode(b);
		if (c == '\uFFFD')
		    return CoderResult.malformedForLength(1);
		if (dl - dp < 1)
		    return CoderResult.OVERFLOW;
		da[dp++] = c;
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
	try {
	    while (src.hasRemaining()) {
		int b = src.get();

		char c = decode(b);
		if (c == '\uFFFD')
		    return CoderResult.malformedForLength(1);
		if (!dst.hasRemaining())
		    return CoderResult.OVERFLOW;
		mark++;
		dst.put(c);
	    }
	    return CoderResult.UNDERFLOW;
	} finally {
	    src.position(mark);
	}
    }

    protected CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
	if (true && src.hasArray() && dst.hasArray())
	    return decodeArrayLoop(src, dst);
	else
	    return decodeBufferLoop(src, dst);
    }

    public char decode(int byteIndex) {
	int n = byteIndex + 128;
	if (n >= byteToCharTable.length() || n < 0)
	    return '\uFFFD';
	return byteToCharTable.charAt(n);
    }
}
