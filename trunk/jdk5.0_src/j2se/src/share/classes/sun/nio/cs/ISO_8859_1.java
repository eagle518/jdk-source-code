/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISO_8859_1.java	1.15 04/03/30
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;
import java.nio.charset.UnmappableCharacterException;


class ISO_8859_1
    extends Charset
    implements HistoricallyNamedCharset
{

    public ISO_8859_1() {
	super("ISO-8859-1", StandardCharsets.aliases_ISO_8859_1);
    }

    public String historicalName() {
	return "ISO8859_1";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof US_ASCII)
		|| (cs instanceof ISO_8859_1));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends CharsetDecoder {

	private Decoder(Charset cs) {
	    super(cs, 1.0f, 1.0f);
	}

	private CoderResult decodeArrayLoop(ByteBuffer src,
					    CharBuffer dst)
	{
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
		    byte b = sa[sp];
		    if (dp >= dl)
			return CoderResult.OVERFLOW;
		    da[dp++] = (char)(b & 0xff);
		    sp++;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }
	}

	private CoderResult decodeBufferLoop(ByteBuffer src,
					     CharBuffer dst)
	{
	    int mark = src.position();
	    try {
		while (src.hasRemaining()) {
		    byte b = src.get();
		    if (!dst.hasRemaining())
			return CoderResult.OVERFLOW;
		    dst.put((char)(b & 0xff));
		    mark++;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}

	protected CoderResult decodeLoop(ByteBuffer src,
					 CharBuffer dst)
	{
	    if (src.hasArray() && dst.hasArray())
		return decodeArrayLoop(src, dst);
	    else
		return decodeBufferLoop(src, dst);
	}

    }

    private static class Encoder extends CharsetEncoder {

	private Encoder(Charset cs) {
	    super(cs, 1.0f, 1.0f);
	}

	public boolean canEncode(char c) {
	    return c <= '\u00FF';
	}

	private final Surrogate.Parser sgp = new Surrogate.Parser();

	private CoderResult encodeArrayLoop(CharBuffer src,
					    ByteBuffer dst)
	{
	    char[] sa = src.array();
	    int sp = src.arrayOffset() + src.position();
	    int sl = src.arrayOffset() + src.limit();
	    assert (sp <= sl);
	    sp = (sp <= sl ? sp : sl);
	    byte[] da = dst.array();
	    int dp = dst.arrayOffset() + dst.position();
	    int dl = dst.arrayOffset() + dst.limit();
	    assert (dp <= dl);
	    dp = (dp <= dl ? dp : dl);
	    try {
		while (sp < sl) {
		    char c = sa[sp];
		    if (c <= '\u00FF') {
			if (dp >= dl)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte)c;
			sp++;
			continue;
		    }
		    if (sgp.parse(c, sa, sp, sl) < 0)
			return sgp.error();
		    return sgp.unmappableResult();
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }
	}

	private CoderResult encodeBufferLoop(CharBuffer src,
					     ByteBuffer dst)
	{
	    int mark = src.position();
	    try {
		while (src.hasRemaining()) {
		    char c = src.get();
		    if (c <= '\u00FF') {
			if (!dst.hasRemaining())
			    return CoderResult.OVERFLOW;
			dst.put((byte)c);
			mark++;
			continue;
		    }
		    if (sgp.parse(c, src) < 0)
			return sgp.error();
		    return sgp.unmappableResult();
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}

	protected CoderResult encodeLoop(CharBuffer src,
					 ByteBuffer dst)
	{
	    if (src.hasArray() && dst.hasArray())
		return encodeArrayLoop(src, dst);
	    else
		return encodeBufferLoop(src, dst);
	}

    }
}
