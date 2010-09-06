/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISO2022_JP.java	1.10 04/06/13
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import sun.nio.cs.HistoricallyNamedCharset;
import sun.nio.cs.Surrogate;
import sun.nio.cs.US_ASCII;

public class ISO2022_JP
    extends Charset
    implements HistoricallyNamedCharset
{
    private static final int ASCII = 0;			// ESC ( B
    private static final int JISX0201_1976 = 1;		// ESC ( J
    private static final int JISX0208_1978 = 2;		// ESC $ @
    private static final int JISX0208_1983 = 3;		// ESC $ B
    private static final int JISX0201_1976_KANA = 4;	// ESC ( I
    private static final int SHIFTOUT = 5;

    public ISO2022_JP() {
	super("ISO-2022-JP", ExtendedCharsets.aliasesFor("ISO-2022-JP"));
    }

    public String historicalName() {
	return "ISO2022JP";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof JIS_X_0201)
		|| (cs instanceof US_ASCII)
		|| (cs instanceof JIS_X_0208)
		|| (cs instanceof ISO2022_JP));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }


    private static class Decoder extends JIS_X_0208_Decoder
	implements DelegatableDecoder {

	private int currentState;

	private Decoder(Charset cs) {
	    super(cs);
	    currentState = ASCII;
	}

	public void implReset() {
	    currentState = ASCII;
        }

	private CoderResult decodeArrayLoop(ByteBuffer src,
					    CharBuffer dst)
	{
	    int previousState = ASCII;
	    int inputSize = 0;
	    int b1 = 0, b2 = 0, b3 = 0; 

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
		    b1 = sa[sp] & 0xff;
		    inputSize = 1;

		    if ((b1 & (byte)0x80) != 0) {
			return CoderResult.malformedForLength(1);
		    }

		    while (b1 == 0x1b || b1 == 0x0e || b1 == 0x0f) {
			if (b1 == 0x1b) {  // ESC  
			    if (sp + inputSize + 2 > sl)
				return CoderResult.UNDERFLOW;
			    b2 = sa[sp + inputSize] & 0xff;
			    inputSize++;
			    if ((b2 & (byte)0x80) != 0) {
				return CoderResult.malformedForLength(1);
			    }
			    if (b2 == (byte)0x28) {  // b2 == ')'
				b3 = sa[sp + inputSize] & 0xff;
				inputSize++;
				if (b3 == 'B'){
				    currentState = ASCII;
				} else if (b3 == 'J'){
				    currentState = JISX0201_1976;
				} else if (b3 == 'I'){
				    currentState = JISX0201_1976_KANA;
				} else {
				    // illegal ESC sequence
				   return CoderResult.malformedForLength(2);
				}
			    } else if (b2 == '$'){
				b3 = sa[sp + inputSize] & 0xff;
				inputSize++;
				if ((b3 & (byte)0x80) != 0) {
				    return CoderResult.malformedForLength(2);
				}
				if (b3 == '@'){
				    currentState = JISX0208_1978;
				} else if (b3 == 'B'){
				    currentState = JISX0208_1983;
				} else {
				    // illegal ESC sequence
				    return CoderResult.malformedForLength(2);
				}
			    }

			if (sp + inputSize + 1 > sl) {
				sp += inputSize;
				return CoderResult.UNDERFLOW;
		        }

			b1 = sa[sp + inputSize] & 0xff;
			inputSize++;
			}
			else if (b1 == 0x0e) {
			    if (sp + inputSize >= sl)
				return CoderResult.UNDERFLOW;

			    previousState = currentState;
			    currentState = SHIFTOUT;
			    b1 = sa[sp + inputSize] & 0xff;
			    inputSize++;
			    if ((b1 & (byte)0x80) != 0) {
				return CoderResult.malformedForLength(1);
			    }
			}
			else if (b1 == 0x0f) {
			if (sp + inputSize >= sl)
			    return CoderResult.UNDERFLOW;
			    currentState = previousState;
			    b1 = sa[sp + inputSize] & 0xff;
			    inputSize++;
			    if ((b1 & (byte)0x80) != 0) {
				return CoderResult.malformedForLength(1);
			    }
			}
		    }
		    if (dp + 1 > dl)
			return CoderResult.OVERFLOW;

		    switch (currentState){
			case ASCII:
			    da[dp++] = (char)(b1 & 0xff);
			    break;
			case JISX0201_1976:
			    switch (b1) {
			      case 0x5c:  // Yen/tilde substitution
				da[dp++] = '\u00a5';
				break;
			      case 0x7e:
				da[dp++] = '\u203e';
				break;
			      default:
				da[dp++] = (char)b1;
				break;
			    }
			    break;
			case JISX0208_1978:
			case JISX0208_1983:
			    if (sp + inputSize + 1> sl)
				return CoderResult.UNDERFLOW;
			    b2 = sa[sp + inputSize] & 0xff;
			    inputSize++;
			    da[dp++] = decodeDouble(b1,b2);
			    break;
			case JISX0201_1976_KANA:
			case SHIFTOUT:
			if (b1 > 0x60) {
			    return CoderResult.malformedForLength(1);
			}
			da[dp++] = (char)(b1 + 0xff40);
			break;
		      }
		      sp += inputSize;
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
	    int b1 = 0, b2 = 0, b3 = 0;
	    int inputSize = 0;
	    int previousState = ASCII;

	    try {
		while (src.hasRemaining()) {
		    b1 = src.get();
		    inputSize = 1;

		    if ((b1 & (byte)0x80) != 0)
			return CoderResult.malformedForLength(1);

		    while (b1 == 0x1b || b1 == 0x0e || b1 == 0x0f) {
			if (b1 == 0x1b) {  // ESC  
			    if (src.remaining() < 1)
				return CoderResult.UNDERFLOW;
			    b2 = src.get() & 0xff;
			    inputSize++;

			    if ((b2 & (byte)0x80) != 0)
				return CoderResult.malformedForLength(2);

			    if (b2 == (byte)0x28) { 
				if (src.remaining() < 2)
				    return CoderResult.UNDERFLOW;
				b3 = src.get() & 0xff;
				inputSize++;
				if (b3 == 'B'){
				    currentState = ASCII;
				} else if (b3 == 'J'){
				    currentState = JISX0201_1976;
				} else if (b3 == 'I'){
				    currentState = JISX0201_1976_KANA;
				} else {
				    // illegal ESC sequence
				   return CoderResult.malformedForLength(3);
				}
			    } else if (b2 == '$'){
				if (src.remaining() < 2)
				    return CoderResult.UNDERFLOW;
				b3 = src.get() & 0xff;
				inputSize++;
				if ((b3 & (byte)0x80) != 0)
				    return CoderResult.malformedForLength(1);
				if (b3 == '@'){
				    currentState = JISX0208_1978;
				} else if (b3 == 'B'){
				    currentState = JISX0208_1983;
				} else {
				    // illegal ESC sequence
				    return CoderResult.malformedForLength(1);
				}
			    }
			b1 = src.get() & 0xff;
			inputSize++;
			}
			else if (b1 == 0x0e) {
			    previousState = currentState;
			    currentState = SHIFTOUT;
			    b1 = src.get() & 0xff;
			    inputSize++;
			    if ((b1 & (byte)0x80) != 0) {
				return CoderResult.malformedForLength(1);
			    }
			}
			else if (b1 == 0x0f) { // shift back in
			    currentState = previousState;
			    b1 = src.get() & 0xff;
			    inputSize++;
			    if ((b1 & (byte)0x80) != 0) {
				return CoderResult.malformedForLength(1);
			    }
			}
		    }

		    if (dst.remaining() < 1)
			return CoderResult.OVERFLOW;

		    switch (currentState){
			case ASCII:
			    dst.put((char)(b1 & 0xff));
			    break;
			case JISX0201_1976:
			    switch (b1) {
			      case 0x5c:  // Yen/tilde substitution
				dst.put('\u00a5');
				break;
			      case 0x7e:
				dst.put('\u203e');
				break;
			      default:
				dst.put((char)b1);
				break;
			    }
			    break;
			case JISX0208_1978:
			case JISX0208_1983:
			    if (src.remaining() < 1)
				return CoderResult.UNDERFLOW;
			    b2 = src.get() & 0xff;
			    inputSize++;
			    dst.put(decodeDouble(b1,b2));
			    break;
			case JISX0201_1976_KANA:
			case SHIFTOUT:
			if (b1 > 0x60) {
			    return CoderResult.malformedForLength(1);
			}
			dst.put((char)(b1 + 0xff40));
			break;
		    }
		    mark += inputSize;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}

	// Make some protected methods public for use by JISAutoDetect
	public CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
	    if (src.hasArray() && dst.hasArray())
		return decodeArrayLoop(src, dst);
	    else
		return decodeBufferLoop(src, dst);
	}

	public CoderResult implFlush(CharBuffer out) {
	    return super.implFlush(out);
	}
    }

    private static class Encoder extends JIS_X_0208_Encoder {

        private int currentMode = ASCII;

	private Encoder(Charset cs) {
	    super(cs, 4.0f, 8.0f);
	}

        protected void implReset() {
            int currentMode = ASCII;
        }

    protected CoderResult implFlush(ByteBuffer out) {
	if (currentMode != ASCII) {
	    if (out.remaining() < 3)
		return CoderResult.OVERFLOW;
	    out.put((byte)0x1b);
	    out.put((byte)0x28);
	    out.put((byte)0x42);
	    currentMode = ASCII;
	}
	return CoderResult.UNDERFLOW;
    }

	public boolean canEncode(char c) {
	    return ((c <= '\u007F') ||
		    (c >= 0xFF61 && c <= 0xFF9F) ||
		    (c == '\u00A5') ||
		    (c == '\u203E') ||
		    (super.canEncode(c)));
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

	    int outputSize = 0;

	    try {
		while (sp < sl) {
		    int newMode = currentMode;
		    char c = sa[sp];

                    if (c <= '\u007F') {
			if (currentMode != ASCII) {
			    if (dl - dp < 4)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)0x1b;
			    da[dp++] = (byte)0x28;
			    da[dp++] = (byte)0x42;
			    da[dp++] = (byte)c;
			    newMode = ASCII;
			    sp++;
			}
			else {
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)c;
			    sp++;
		        }
		    }
		    // Is it a single byte kana?
		    else if (c >= 0xff61 && c <= 0xff9f) {
			if (currentMode != JISX0201_1976_KANA) {
			    if (dl - dp < 4)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)0x1b;
			    da[dp++] = (byte)0x28;
			    da[dp++] = (byte)0x49;
			    da[dp++] = (byte)(c - 0xff40);
			    newMode = JISX0201_1976_KANA;
			    sp++;
			} else {
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)(c - 0xff40);
			    sp++;
			}
		    }
		    else if (c == '\u00A5') {
			if (currentMode != JISX0201_1976) {
			    if (dl - dp < 4)
				return CoderResult.OVERFLOW;
			    da[dp++] = (byte)0x1b;
			    da[dp++] = (byte)0x28;
			    da[dp++] = (byte)0x4a;
			    da[dp++] = (byte)0x5c;
			    newMode = JISX0201_1976;
			    sp++;
			} else {
			    da[dp++] = (byte)0x5C;
			    sp++;
			}
		    }
		    else if (c == '\u203E') { // is it a tilde?
			    if (currentMode != JISX0201_1976) {
			    if (dl - dp < 4)
				return CoderResult.OVERFLOW;
				da[dp++] = (byte)0x1b;
				da[dp++] = (byte)0x28;
				da[dp++] = (byte)0x4a;
				da[dp++] = (byte)0x7e;
				newMode = JISX0201_1976;
				sp++;
			    } else {
				if (dl - dp < 1)
				    return CoderResult.OVERFLOW;
				da[dp++] = (byte)0x7e;
				sp++;
			    }
		    }
		    else {
			if (dl - dp < 5)
			    return CoderResult.OVERFLOW;
			int index = encodeDouble(c);
			if (index != 0) {
			    if (currentMode != JISX0208_1983) {
				da[dp++] = (byte)0x1b;
				da[dp++] = (byte)0x24;
				da[dp++] = (byte)0x42;
				da[dp++] = (byte)(index >> 8);
				da[dp++] = (byte)(index & 0xff);
				newMode = JISX0208_1983;
				sp++;
			    } else {
				da[dp++] = (byte)(index >> 8);
				da[dp++] = (byte)(index & 0xff);
				sp++;
			    }
			}
			else { 
			    return CoderResult.unmappableForLength(1);
			}
		    }

		    if (Surrogate.is(c)) {
			if (sgp.parse(c, sa, sp, sl) < 0)
			    return sgp.error();
			return sgp.unmappableResult();
		    }

		    if (dl - dp < outputSize)
			return CoderResult.OVERFLOW;
		    currentMode = newMode; 

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
		    int newMode = currentMode;

		    char c = src.get();

                    if (c <= '\u007F') {
			if (currentMode != ASCII) {
			    if (dst.remaining() < 4)
				return CoderResult.OVERFLOW;
			    dst.put((byte)0x1b);
			    dst.put((byte)0x28);
			    dst.put((byte)0x42);
			    dst.put((byte)c);
			    mark++;
			    newMode = ASCII;
			}
			else {
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    dst.put((byte)c);
			    mark++;
		        }
		    }
		    // Is it a single byte kana?
		    else if (c >= 0xff61 && c <= 0xff9f) {
			if (currentMode != JISX0201_1976_KANA) {
			    if (dst.remaining() < 4)
				return CoderResult.OVERFLOW;
			    dst.put((byte)0x1b);
			    dst.put((byte)0x28);
			    dst.put((byte)0x49);
			    dst.put((byte)(c - 0xff40));
			    mark++;
			    newMode = JISX0201_1976_KANA;
			} else {
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    mark++;
			    dst.put((byte)(c - 0xff40));
			}
		    }
		    else if (c == '\u00a5') {
			if (currentMode != JISX0201_1976) {
			    if (dst.remaining() < 4)
				return CoderResult.OVERFLOW;
			    dst.put((byte)0x1b);
			    dst.put((byte)0x28);
			    dst.put((byte)0x4a);
			    dst.put((byte)0x5c);
			    mark++;
			    newMode = JISX0201_1976;
			} else {
			    mark++;
			    dst.put((byte)0x5c);
			}
		    }
		    else if (c == '\u203e') { // is it a tilde?
			    if (currentMode != JISX0201_1976) {
				if (dst.remaining() < 4)
				    return CoderResult.OVERFLOW;
				dst.put((byte)0x1b);
				dst.put((byte)0x28);
				dst.put((byte)0x4a);
				dst.put((byte)0x7e);
			        mark++;
				newMode = JISX0201_1976;
			    } else {
				if (dst.remaining() < 1)
				    return CoderResult.OVERFLOW;
			 	dst.put((byte)0x7e);	
			        mark++;
			    }
		    }
		    else {
			if (dst.remaining() < 5)
			    return CoderResult.OVERFLOW;
			int index = encodeDouble(c);
			if (index != 0) {
			    if (currentMode != JISX0208_1983) {
				dst.put((byte)0x1b);
				dst.put((byte)0x24);
				dst.put((byte)0x42);
				dst.put((byte)(index >> 8));
				dst.put((byte)(index & 0xff));
				newMode = JISX0208_1983;
			        mark++;
			    } else {
				dst.put((byte)(index >> 8));
				dst.put((byte)(index & 0xff));
			        mark++;
			    }
			}
			else { 
			    return CoderResult.unmappableForLength(1);
			}
		    }

		    if (Surrogate.is(c)) {
			if (sgp.parse(c, src) < 0)
			    return sgp.error();
			return sgp.unmappableResult();
		    }
		currentMode = newMode;
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
