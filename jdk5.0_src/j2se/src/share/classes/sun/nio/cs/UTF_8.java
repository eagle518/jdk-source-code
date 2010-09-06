/*
 * @(#)UTF_8.java	1.22 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;
import java.nio.charset.UnmappableCharacterException;


/*
 * # Bits   Bit pattern
 * 1    7   0xxxxxxx
 * 2   11   110xxxxx 10xxxxxx
 * 3   16   1110xxxx 10xxxxxx 10xxxxxx
 * 4   21   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 5   26   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 6   31   1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * UCS-2 uses 1-3, UTF-16 uses 1-4, UCS-4 uses 1-6
 */

class UTF_8
    extends Charset
    implements HistoricallyNamedCharset
{

    public UTF_8() {
	super("UTF-8", StandardCharsets.aliases_UTF_8);
    }

    public String historicalName() {
	return "UTF8";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof US_ASCII)
		|| (cs instanceof ISO_8859_1)
		|| (cs instanceof ISO_8859_15)
		|| (cs instanceof MS1252)
		|| (cs instanceof UTF_8)
		|| (cs instanceof UTF_16)
		|| (cs instanceof UTF_16BE)
	        || (cs.name().equals("GBK"))
	        || (cs.name().equals("GB18030"))
	        || (cs.name().equals("ISO-8859-2"))
	        || (cs.name().equals("ISO-8859-3"))
	        || (cs.name().equals("ISO-8859-4"))
	        || (cs.name().equals("ISO-8859-5"))
	        || (cs.name().equals("ISO-8859-6"))
	        || (cs.name().equals("ISO-8859-7"))
	        || (cs.name().equals("ISO-8859-8"))
	        || (cs.name().equals("ISO-8859-9"))
	        || (cs.name().equals("ISO-8859-13"))
	        || (cs.name().equals("windows-1251"))
	        || (cs.name().equals("windows-1253"))
	        || (cs.name().equals("windows-1254"))
	        || (cs.name().equals("windows-1255"))
	        || (cs.name().equals("windows-1256"))
	        || (cs.name().equals("windows-1257"))
	        || (cs.name().equals("windows-1258"))
	        || (cs.name().equals("windows-932"))
	        || (cs.name().equals("x-mswin-936"))
	        || (cs.name().equals("x-windows-949"))
	        || (cs.name().equals("x-windows-950"))
	        || (cs.name().equals("windows-31j"))
	        || (cs.name().equals("JIS_X0201"))
	        || (cs.name().equals("x-JIS0208"))
	        || (cs.name().equals("JIS_X0212-1990"))
	        || (cs.name().equals("GB2312"))
	        || (cs.name().equals("EUC-KR"))
	        || (cs.name().equals("x-EUC-TW"))
	        || (cs.name().equals("EUC-JP"))
	        || (cs.name().equals("x-euc-jp-linux"))
	        || (cs.name().equals("KOI8-R"))
	        || (cs.name().equals("TIS-620"))
	        || (cs.name().equals("x-ISCII91"))
	        || (cs.name().equals("Big5"))
	        || (cs.name().equals("Big5-HKSCS"))
	        || (cs.name().equals("x-MS950-HKSCS"))
	        || (cs.name().equals("ISO-2022-JP"))
	        || (cs.name().equals("ISO-2022-KR"))
	        || (cs.name().equals("x-ISO-2022-CN-CNS"))
	        || (cs.name().equals("x-ISO-2022-CN-GB"))
	        || (cs.name().equals("Big5-HKSCS"))
	        || (cs.name().equals("x-Johab"))
	        || (cs.name().equals("Shift_JIS"))
		|| (cs instanceof UTF_16LE));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }


    private static class Decoder extends CharsetDecoder {

	private Decoder(Charset cs) {
	    super(cs, 1.0f, 2.0f);
	}

	private boolean isContinuation(int b) {
	    return ((b & 0xc0) == 0x80);
	}

	private final Surrogate.Generator sgg = new Surrogate.Generator();

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
		    int b1 = sa[sp];
		    int b2, b3;
		    switch ((b1 >> 4) & 0x0f) {

		    case 0: case 1: case 2: case 3:
		    case 4: case 5: case 6: case 7:
			// 1 byte, 7 bits: 0xxxxxxx
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			da[dp++] = (char)(b1 & 0x7f);
			sp++;
			continue;

		    case 12: case 13:
			// 2 bytes, 11 bits: 110xxxxx 10xxxxxx
			if (sl - sp < 2)
			    return CoderResult.UNDERFLOW;
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			if (!isContinuation(b2 = sa[sp + 1]))
			    return CoderResult.malformedForLength(1);
			da[dp++] = ((char)(((b1 & 0x1f) << 6) |
					   ((b2 & 0x3f) << 0)));
			sp += 2;
			continue;

		    case 14:
			// 3 bytes, 16 bits: 1110xxxx 10xxxxxx 10xxxxxx
			if (sl - sp < 3)
			    return CoderResult.UNDERFLOW;
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			if (!isContinuation(b2 = sa[sp + 1]))
			    return CoderResult.malformedForLength(1);
			if (!isContinuation(b3 = sa[sp + 2]))
			    return CoderResult.malformedForLength(2);
			da[dp++] = ((char)(((b1 & 0x0f) << 12) |
					   ((b2 & 0x3f) << 06) |
					   ((b3 & 0x3f) << 0)));
			sp += 3;
			continue;

		    case 15:
			// 4, 5, or 6 bytes

			int b4, b5, b6, uc, n;
			switch (b1 & 0x0f) {

			case 0: case 1: case 2: case 3:
			case 4: case 5: case 6: case 7:
			    // 4 bytes, 21 bits
			    if (sl - sp < 4)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = sa[sp + 1]))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = sa[sp + 2]))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = sa[sp + 3]))
				return CoderResult.malformedForLength(3);
			    uc = (((b1 & 0x07) << 18) |
				  ((b2 & 0x3f) << 12) |
				  ((b3 & 0x3f) << 06) |
				  ((b4 & 0x3f) << 00));
			    n = 4;
			    break;

			case 8: case 9: case 10: case 11:
			    // 5 bytes, 26 bits
			    if (sl - sp < 5)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = sa[sp + 1]))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = sa[sp + 2]))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = sa[sp + 3]))
				return CoderResult.malformedForLength(3);
			    if (!isContinuation(b5 = sa[sp + 4]))
				return CoderResult.malformedForLength(4);
			    uc = (((b1 & 0x03) << 24) |
				  ((b2 & 0x3f) << 18) |
				  ((b3 & 0x3f) << 12) |
				  ((b4 & 0x3f) << 06) |
				  ((b5 & 0x3f) << 00));
			    n = 5;
			    break;

			case 12: case 13:
			    // 6 bytes, 31 bits
			    if (sl - sp < 6)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = sa[sp + 1]))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = sa[sp + 2]))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = sa[sp + 3]))
				return CoderResult.malformedForLength(3);
			    if (!isContinuation(b5 = sa[sp + 4]))
				return CoderResult.malformedForLength(4);
			    if (!isContinuation(b6 = sa[sp + 5]))
				return CoderResult.malformedForLength(5);
			    uc = (((b1 & 0x01) << 30) |
				  ((b2 & 0x3f) << 24) |
				  ((b3 & 0x3f) << 18) |
				  ((b4 & 0x3f) << 12) |
				  ((b5 & 0x3f) << 06) |
				  ((b6 & 0x3f)));
			    n = 6;
			    break;

			default:
			    return CoderResult.malformedForLength(1);

			}

			int gn = sgg.generate(uc, n, da, dp, dl);
			if (gn < 0)
			    return sgg.error();
			dp += gn;
			sp += n;
			continue;

		    default:
			return CoderResult.malformedForLength(1);

		    }

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
		    int b1 = src.get();
		    int b2, b3;
		    switch ((b1 >> 4) & 0x0f) {

		    case 0: case 1: case 2: case 3:
		    case 4: case 5: case 6: case 7:
			// 1 byte, 7 bits: 0xxxxxxx
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			dst.put((char)b1);
			mark++;
			continue;

		    case 12: case 13:
			// 2 bytes, 11 bits: 110xxxxx 10xxxxxx
			if (src.remaining() < 1)
			    return CoderResult.UNDERFLOW;
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			if (!isContinuation(b2 = src.get()))
			    return CoderResult.malformedForLength(1);
			dst.put((char)(((b1 & 0x1f) << 6) |
				       ((b2 & 0x3f) << 0)));
			mark += 2;
			continue;

		    case 14:
			// 3 bytes, 16 bits: 1110xxxx 10xxxxxx 10xxxxxx
			if (src.remaining() < 2)
			    return CoderResult.UNDERFLOW;
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			if (!isContinuation(b2 = src.get()))
			    return CoderResult.malformedForLength(1);
			if (!isContinuation(b3 = src.get()))
			    return CoderResult.malformedForLength(2);
			dst.put((char)(((b1 & 0x0f) << 12) |
				       ((b2 & 0x3f) << 06) |
				       ((b3 & 0x3f) << 0)));
			mark += 3;
			continue;

		    case 15:
			// 4, 5, or 6 bytes

			int b4, b5, b6, uc, n;
			switch (b1 & 0x0f) {

			case 0: case 1: case 2: case 3:
			case 4: case 5: case 6: case 7:
			    // 4 bytes, 21 bits
			    if (src.remaining() < 3)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = src.get()))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = src.get()))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = src.get()))
				return CoderResult.malformedForLength(3);
			    uc = (((b1 & 0x07) << 18) |
				  ((b2 & 0x3f) << 12) |
				  ((b3 & 0x3f) << 06) |
				  ((b4 & 0x3f) << 00));
			    n = 4;
			    break;

			case 8: case 9: case 10: case 11:
			    // 5 bytes, 26 bits
			    if (src.remaining() < 4)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = src.get()))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = src.get()))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = src.get()))
				return CoderResult.malformedForLength(3);
			    if (!isContinuation(b5 = src.get()))
				return CoderResult.malformedForLength(4);
			    uc = (((b1 & 0x03) << 24) |
				  ((b2 & 0x3f) << 18) |
				  ((b3 & 0x3f) << 12) |
				  ((b4 & 0x3f) << 06) |
				  ((b5 & 0x3f) << 00));
			    n = 5;
			    break;

			case 12: case 13:
			    // 6 bytes, 31 bits
			    if (src.remaining() < 4)
				return CoderResult.UNDERFLOW;
			    if (!isContinuation(b2 = src.get()))
				return CoderResult.malformedForLength(1);
			    if (!isContinuation(b3 = src.get()))
				return CoderResult.malformedForLength(2);
			    if (!isContinuation(b4 = src.get()))
				return CoderResult.malformedForLength(3);
			    if (!isContinuation(b5 = src.get()))
				return CoderResult.malformedForLength(4);
			    if (!isContinuation(b6 = src.get()))
				return CoderResult.malformedForLength(5);
			    uc = (((b1 & 0x01) << 30) |
				  ((b2 & 0x3f) << 24) |
				  ((b3 & 0x3f) << 18) |
				  ((b4 & 0x3f) << 12) |
				  ((b5 & 0x3f) << 06) |
				  ((b6 & 0x3f)));
			    n = 6;
			    break;

			default:
			    return CoderResult.malformedForLength(1);

			}

			if (sgg.generate(uc, n, dst) < 0)
			    return sgg.error();
			mark += n;
			continue;

		    default:
			return CoderResult.malformedForLength(1);

		    }

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
	    super(cs, 1.1f, 4.0f);
	}

	public boolean canEncode(char c) {
	    return !Surrogate.is(c);
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

		    if (c < 0x80) {
			// Have at most seven bits
			if (dp >= dl)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte)c;
			sp++;
			continue;
		    }

		    if (!Surrogate.is(c)) {
			// 2 bytes, 11 bits
			if (c < 0x800) {
			    if (dl - dp < 2)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)(0xc0 | ((c >> 06)));
			    da[dp++] = (byte)(0x80 | ((c >> 00) & 0x3f));
			    sp++;
			    continue;
			}
			if (c <= '\uFFFF') {
			    // 3 bytes, 16 bits
			    if (dl - dp < 3)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)(0xe0 | ((c >> 12)));
			    da[dp++] = (byte)(0x80 | ((c >> 06) & 0x3f));
			    da[dp++] = (byte)(0x80 | ((c >> 00) & 0x3f));
			    sp++;
			    continue;
			}
		    }

		    // Have a surrogate pair
		    int uc = sgp.parse(c, sa, sp, sl);
		    if (uc < 0)
			return sgp.error();
		    if (uc < 0x200000) {
			if (dl - dp < 4)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte)(0xf0 | ((uc >> 18)));
			da[dp++] = (byte)(0x80 | ((uc >> 12) & 0x3f));
			da[dp++] = (byte)(0x80 | ((uc >> 06) & 0x3f));
			da[dp++] = (byte)(0x80 | ((uc >> 00) & 0x3f));
			sp += sgp.increment();
			continue;
		    }
		    assert false;

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

		    if (c < 0x80) {
			// Have at most seven bits
			if (!dst.hasRemaining())
			    return CoderResult.OVERFLOW;
			dst.put((byte)c);
			mark++;
			continue;
		    }

		    if (!Surrogate.is(c)) {
			if (c < 0x800) {
			    // 2 bytes, 11 bits
			    if (dst.remaining() < 2)
				return CoderResult.OVERFLOW;
			    dst.put((byte)(0xc0 | ((c >> 06))));
			    dst.put((byte)(0x80 | ((c >> 00) & 0x3f)));
			    mark++;
			    continue;
			}
			if (c <= '\uFFFF') {
			    // 3 bytes, 16 bits
			    if (dst.remaining() < 3)
				return CoderResult.OVERFLOW;
			    dst.put((byte)(0xe0 | ((c >> 12))));
			    dst.put((byte)(0x80 | ((c >> 06) & 0x3f)));
			    dst.put((byte)(0x80 | ((c >> 00) & 0x3f)));
			    mark++;
			    continue;
			}
		    }

		    // Have a surrogate pair
		    int uc = sgp.parse(c, src);
		    if (uc < 0)
			return sgp.error();
		    if (uc < 0x200000) {
			if (dst.remaining() < 4)
			    return CoderResult.OVERFLOW;
			dst.put((byte)(0xf0 | ((uc >> 18))));
			dst.put((byte)(0x80 | ((uc >> 12) & 0x3f)));
			dst.put((byte)(0x80 | ((uc >> 06) & 0x3f)));
			dst.put((byte)(0x80 | ((uc >> 00) & 0x3f)));
			mark += sgp.increment();
			continue;
		    }
		    assert false;

		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}

	protected final CoderResult encodeLoop(CharBuffer src,
					       ByteBuffer dst)
	{
	    if (src.hasArray() && dst.hasArray())
		return encodeArrayLoop(src, dst);
	    else
		return encodeBufferLoop(src, dst);
	}

    }

}
