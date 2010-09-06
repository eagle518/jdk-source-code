/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISO2022_CN.java	1.3	03/12/19
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import sun.nio.cs.HistoricallyNamedCharset;
import sun.nio.cs.US_ASCII;

public class ISO2022_CN
    extends Charset
    implements HistoricallyNamedCharset
{
    private static final byte ISO_ESC = 0x1b;
    private static final byte ISO_SI = 0x0f;
    private static final byte ISO_SO = 0x0e;
    private static final byte ISO_SS2_7 = 0x4e;
    private static final byte ISO_SS3_7 = 0x4f;
    private static final byte MSB = (byte)0x80;
    private static final char REPLACE_CHAR = '\uFFFD';

    private static final byte SODesig = 0;
    private static final byte SODesigCNS = 1;
    private static final byte SS2Desig = 2;
    private static final byte SS3Desig = 3;

    private static CharsetDecoder gb2312Decoder = null;
    private static CharsetDecoder cnsDecoder = null;

    public ISO2022_CN() {
	super("ISO-2022-CN", ExtendedCharsets.aliasesFor("ISO-2022-CN"));
    }

    public String historicalName() {
	return "ISO2022CN";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof EUC_CN)     // GB2312-80 repertoire 
		|| (cs instanceof US_ASCII)
		|| (cs instanceof EUC_TW)  // CNS11643 repertoire
		|| (cs instanceof ISO2022_CN));
    }

    public CharsetDecoder newDecoder() {
	gb2312Decoder = new EUC_CN().newDecoder();
	cnsDecoder = new EUC_TW().newDecoder();
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	throw new UnsupportedOperationException();
    }

    public boolean canEncode() {
	return false;
    }

    private static class Decoder extends CharsetDecoder {

	private boolean shiftOut;
	private byte currentDesig;

	private Decoder(Charset cs) {
	    super(cs, 1.0f, 1.0f);
	    shiftOut = false;
	    currentDesig = SODesig;
	}

	protected void implReset() {
	    shiftOut= false;
	    currentDesig = SODesig;
        }

	private char cnsDecode(byte byte1, byte byte2,
			       byte byte3, byte byte4) { 

	    byte3 |= MSB;
	    byte4 |= MSB;
	    CharBuffer cBuf = CharBuffer.allocate(1);
	    ByteBuffer bBuf = ByteBuffer.allocate(4);

	    try {
		bBuf.put(byte1);
		bBuf.put(byte2);
		bBuf.put(byte3);
		bBuf.put(byte4);
		bBuf.flip();
		cBuf = cnsDecoder.decode(bBuf);
	    } catch (CharacterCodingException e) { return REPLACE_CHAR; }

	    return cBuf.get();
        }

	private char getUnicode(byte byte1, byte byte2, byte shiftFlag) {
	    byte1 |= MSB;
	    byte2 |= MSB;

	    char result = REPLACE_CHAR;
	    CharBuffer cBuf = CharBuffer.allocate(1);
	    ByteBuffer bBuf = ByteBuffer.allocate(4);

	    switch(shiftFlag) {
	    case SODesig:
	    case SS2Desig:	
		try {
		    bBuf.put(byte1);
		    bBuf.put(byte2);
		    bBuf.flip();
		    cBuf = gb2312Decoder.decode(bBuf);
		    return cBuf.get();
		}
		catch (CharacterCodingException e) {
			result =  REPLACE_CHAR;
		}
		break;
	    case SODesigCNS:	
		try {
		    bBuf.put(byte1);
		    bBuf.put(byte2);
		    bBuf.flip();
		    cBuf = cnsDecoder.decode(bBuf);
		    return cBuf.get();
		}
		catch (CharacterCodingException e) {
		    result = REPLACE_CHAR;
		}
		break;
	    default:
		result = REPLACE_CHAR;
	    }
	    return result;
	}
	private CoderResult decodeBufferLoop(ByteBuffer src,
					     CharBuffer dst)
	{
	    int mark = src.position();
	    byte b1 = 0, b2 = 0, b3 = 0, b4 = 0;
	    int inputSize = 0;

	    try {
		while (src.hasRemaining()) {
		    b1 = src.get();
		    inputSize = 1;

		    while (b1 == ISO_ESC ||
		           b1 == ISO_SO ||
		           b1 == ISO_SI) {
			if (b1 == ISO_ESC) {  // ESC  
			    currentDesig = SODesig;

			    if (src.remaining() < 1)
				return CoderResult.UNDERFLOW;

			    b2 = src.get();
			    inputSize++;

			    if ((b2 & (byte)0x80) != 0)
				return CoderResult.malformedForLength(inputSize);

			    if (b2 == (byte)0x24) { 
				if (src.remaining() < 1)
				    return CoderResult.UNDERFLOW;

				b3 = src.get();
				inputSize++;

			        if ((b3 & (byte)0x80) != 0)
				    return CoderResult.malformedForLength(inputSize);
				if (b3 == 'A'){		     // "$A"
				    currentDesig = SODesig;
				} else if (b3 == ')') {
				    if (src.remaining() < 1)
					return CoderResult.UNDERFLOW;
				    b4 = src.get();
				    inputSize++;

				    if (b4 == 'A'){          // "$)A"
					currentDesig = SODesig;
				    } else if (b4 == 'G'){   // "$)G"
				        currentDesig = SODesigCNS;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else if (b3 == '*') {
				    if (src.remaining() < 1)
					return CoderResult.UNDERFLOW;
				    b4 = src.get();
				    inputSize++;
				    if (b4 == 'H'){	     // "$*H"
				        currentDesig = SS2Desig;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else if (b3 == '+') {
				    if (src.remaining() < 1)
					return CoderResult.UNDERFLOW;
				    b4 = src.get();
				    inputSize++;
				    if (b4 == 'I'){	     // "$+I"
				        currentDesig = SS3Desig;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else {
					return CoderResult.malformedForLength(inputSize);
				}
			    } else if (b2 == ISO_SS2_7 || b2 == ISO_SS3_7) {
				if (src.remaining() < 2)
				    return CoderResult.UNDERFLOW;
				b3 = src.get();
				b4 = src.get();
				inputSize += 2;
				b2 = (b2 == ISO_SS2_7) ? (byte)0xa2:
							 (byte)0xa3;
				if (dst.remaining() < 1)
				    return CoderResult.OVERFLOW;

				dst.put(cnsDecode((byte)0x8e,
                                                  (byte)b2,
						  (byte)b3,
                                                  (byte)b4));
			    } else {
				return CoderResult.malformedForLength(inputSize);
			    }
		        } else if (b1 == ISO_SO) {
			    shiftOut = true;
			} else if (b1 == ISO_SI) { // shift back in
			    shiftOut = false;
			}
			if (src.remaining() < 1)
			    return CoderResult.UNDERFLOW;
			b1 = src.get();
			inputSize++;
		    }

		    if (dst.remaining() < 1)
			return CoderResult.OVERFLOW;

		    if (!shiftOut) {
			dst.put((char)b1);
			mark += inputSize;
		    } else { 
			switch (currentDesig){
			    case SODesigCNS:
				if (src.remaining() < 1)
				    return CoderResult.UNDERFLOW;
				b2 = src.get();
				inputSize++;
				dst.put(getUnicode((byte)b1,
						   (byte)b2,
						    currentDesig));
				break;
			    case SS2Desig:
				if (src.remaining() < 1)
				    return CoderResult.UNDERFLOW;
				b2 = src.get();
				inputSize++;
				dst.put(cnsDecode((byte)0x8e,
						   ISO_SS2_7,
					 	   (byte)b2,
						    SS2Desig));
				break;
			    case SS3Desig:
				break;
			    case SODesig:
			    default:
				if (src.remaining() < 1)
				    return CoderResult.UNDERFLOW;
				b2 = src.get();
				inputSize++;
				currentDesig = SODesig;
				dst.put(getUnicode((byte)b1,
						   (byte)b2,
						    currentDesig));	
			        break;
			}
		    mark += inputSize;
		    }
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}

	private CoderResult decodeArrayLoop(ByteBuffer src,
					    CharBuffer dst)
	{

	    int inputSize = 0;
	    byte b1 = 0, b2 = 0, b3 = 0, b4 = 0;

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
		    b1 = sa[sp];
		    inputSize = 1;

		    while (b1 == ISO_ESC || b1 == ISO_SO || b1 == ISO_SI) {
			if (b1 == ISO_ESC) {  // ESC  
			    currentDesig = SODesig;

			    if (sp + 2 > sl)
				return CoderResult.UNDERFLOW;

			    b2 = sa[sp + 1];
			    inputSize++;

			    if ((b2 & (byte)0x80) != 0)
				return CoderResult.malformedForLength(inputSize);
			    if (b2 == (byte)0x24) { 
				if (sp + 3 > sl)
				    return CoderResult.UNDERFLOW;

				b3 = sa[sp + 2];
				inputSize++;

			        if ((b3 & (byte)0x80) != 0)
				    return CoderResult.malformedForLength(inputSize);
				if (b3 == 'A'){		     // "$A"
				    currentDesig = SODesig;
				} else if (b3 == ')') {
				    if (sp + 4 > sl)
					return CoderResult.UNDERFLOW;
				    b4 = sa[sp + 3];
				    inputSize++;

				    if (b4 == 'A'){          // "$)A"
					currentDesig = SODesig;
				    } else if (b4 == 'G'){   // "$)G"
				        currentDesig = SODesigCNS;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else if (b3 == '*') {
				    if (sp + 4 > sl)
					return CoderResult.UNDERFLOW;
				    b4 = sa[sp + 3];
				    inputSize++;
				    if (b4 == 'H'){	     // "$*H"
				        currentDesig = SS2Desig;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else if (b3 == '+') {
				    if (sp + 4 > sl) 
					return CoderResult.UNDERFLOW;
				    b4 = sa[sp + 3];
				    inputSize++;
				    if (b4 == 'I'){	     // "$+I"
				        currentDesig = SS3Desig;
				    } else {
					return CoderResult.malformedForLength(inputSize);
				    }
				} else {
					return CoderResult.malformedForLength(inputSize);
				}
			    } else if (b2 == ISO_SS2_7 || b2 == ISO_SS3_7) {
				if (sp + 4 > sl) {
				    return CoderResult.UNDERFLOW;
				}
				b3 = sa[sp + 2];
				b4 = sa[sp + 3];
				b2 = (b2 == ISO_SS2_7) ? (byte)0xa2:
							 (byte)0xa3;
				if (dl - dp < 1)  {
				    return CoderResult.OVERFLOW;
				}

				da[dp++] = cnsDecode((byte)0x8e,
                                                  (byte)b2,
						  (byte)b3,
                                                  (byte)b4);
				sp += 4;
			    } else {
				return CoderResult.malformedForLength(inputSize);
			    }
		        } else if (b1 == ISO_SO) {
			    shiftOut = true;
			} else if (b1 == ISO_SI) { // shift back in
			    shiftOut = false;
			}

			if (sp + inputSize + 1 > sl)
			    return CoderResult.UNDERFLOW;
			b1 = sa[sp + inputSize];

			sp += inputSize;
			inputSize = 1;

		    }

		    if (dl - dp < 1) {
			return CoderResult.OVERFLOW;
		    }
		    inputSize = 1;

		    if (!shiftOut) {
			da[dp++] = (char)(b1);
		    } else { 
			// more complex state 
			switch (currentDesig){
			    case SODesigCNS:
				if (sp + 2 > sl)
				    return CoderResult.UNDERFLOW;
				b2 = sa[sp + 1];
				inputSize++;
				da[dp++] = getUnicode((byte)b1,
						      (byte)b2,
						       currentDesig);
				break;
			    case SS2Desig:
				if (sp + 2 > sl)
				    return CoderResult.UNDERFLOW;
				b2 = sa[sp + 1];
				inputSize++;
				da[dp++] = cnsDecode((byte)0x8e,
					 	       ISO_SS2_7,
						       b1,
						       b2);
				break;
			    case SS3Desig:
				break;
			    case SODesig:
			    default:
				if (sp + 2 > sl) 
				    return CoderResult.UNDERFLOW;
				b2 = sa[sp + 1];
				inputSize++;
				currentDesig = SODesig;
				da[dp++] = getUnicode((byte)b1,
						      (byte)b2,
						       currentDesig);	
			        break;
			}
		    }
		    sp += inputSize;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
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
}
