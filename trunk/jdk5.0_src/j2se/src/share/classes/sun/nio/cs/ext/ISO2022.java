/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISO2022.java	1.5 04/06/14
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

abstract class ISO2022
    extends Charset
{

    private static final byte ISO_ESC = 0x1b;
    private static final byte ISO_SI = 0x0f;
    private static final byte ISO_SO = 0x0e;
    private static final byte ISO_SS2_7 = 0x4e;
    private static final byte ISO_SS3_7 = 0x4f;
    private static final byte MSB = (byte)0x80;
    private static final char REPLACE_CHAR = '\uFFFD';
    private static final byte maxDesignatorLength = 3; 

    public ISO2022(String csname, String[] aliases) {
	super(csname, aliases);
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends CharsetDecoder {

	// Value to be filled by subclass
	protected String SODesig[];
	protected String SS2Desig[] = null;
	protected String SS3Desig[] = null;

	protected CharsetDecoder SODecoder[];
	protected CharsetDecoder SS2Decoder[] = null;
	protected CharsetDecoder SS3Decoder[] = null;

	private static final byte SOFlag = 0;
	private static final byte SS2Flag = 1;
	private static final byte SS3Flag = 2;

	private int curSODes, curSS2Des, curSS3Des;
	private boolean shiftout;
	private CharsetDecoder tmpDecoder[];

	protected Decoder(Charset cs) {
	    super(cs, 1.0f, 1.0f);
	}

	protected void implReset() {
	    curSODes = 0;
	    curSS2Des = 0;
	    curSS3Des = 0;
	    shiftout = false;
	}

	private char encode(byte byte1, byte byte2, byte shiftFlag)
	{
	    byte1 |= MSB;
	    byte2 |= MSB;

	    byte[] tmpByte = { byte1,byte2 };
	    char[] tmpChar = new char[1];
	    int     i = 0,
		    tmpIndex = 0;

	    switch(shiftFlag) {
	    case SOFlag:
		tmpIndex = curSODes;
		tmpDecoder = (CharsetDecoder [])SODecoder;
		break;
	    case SS2Flag:	
		tmpIndex = curSS2Des;
		tmpDecoder = (CharsetDecoder [])SS2Decoder;
		break;
	    case SS3Flag:
		tmpIndex = curSS3Des;
		tmpDecoder = (CharsetDecoder [])SS3Decoder;
		break;
	    }

	    for(i = 0; i < tmpDecoder.length; i++) {
		if(tmpIndex == i) {
		    try {
			ByteBuffer bb = ByteBuffer.wrap(tmpByte,0,2);
			CharBuffer cc = CharBuffer.wrap(tmpChar,0,1);
			tmpDecoder[i].decode(bb, cc, true);
			cc.flip();
			return cc.get();
		    } catch (Exception e) {}
		}
	    }
	    return REPLACE_CHAR;
	}


	private CoderResult decodeArrayLoop(ByteBuffer src,
					    CharBuffer dst)
	{
	    int DesignatorLength = 0;

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

	    int b1 = 0, b2 = 0, b3 = 0;
	    int inputSize = 1;
	    int i;

	    try {
		while (sp < sl) {
		    b1 = sa[sp] & 0xff;
		    switch (b1) {
			case ISO_SO:
			    shiftout = true;
			    inputSize = 1;
			    break;
			case ISO_SI:
			    shiftout = false;
			    inputSize = 1;
			    break;
			case ISO_ESC:
			    int size =
				Math.min(maxDesignatorLength, 
					 src.remaining());
			    byte[] tmpBuf = new byte[size];

			    if (sl - sp < (size + 1))
				return CoderResult.UNDERFLOW;

			    for (int o = 0 ; o < size; o++) {
				tmpBuf[o] = (byte)sa[sp + o + 1]; 
			    } 

			    String tmpString =
				    new String(tmpBuf, 0, size);
			    for (i = 0; i < SODesig.length; i++) {
				if(tmpString.indexOf(SODesig[i])
							== 0) {
				    curSODes = i;
				    DesignatorLength =
					    SODesig[i].length();
			            inputSize = DesignatorLength + 1;
				    break;
				}
			    }

			    if (i == SODesig.length) {
				for (i = 0; i < SS2Desig.length; i++) {
				    if (tmpString.indexOf(SS2Desig[i]) == 0) {
					curSS2Des = i;
					DesignatorLength = SS2Desig[i].length();
					break;
				    }
				}
				if(i == SS2Desig.length) {
				    for(i = 0; i < SS3Desig.length; i++) {
					if (tmpString.indexOf(SS3Desig[i]) == 0) {
					    curSS3Des = i;
					    DesignatorLength = SS3Desig[i].length();
					    break;
					}
				    }
				    if (i == SS3Desig.length) {
					switch(b1) {
					case ISO_SS2_7:
					    if (sl - sp < 3)
						return CoderResult.UNDERFLOW;
					    b2 = sa[sp +1];
					    b3 = sa[sp +1];
					    if (dl - dp <1)
						return CoderResult.OVERFLOW;
					    da[dp] = encode((byte)b2,
								(byte)b3,
								SS2Flag);
					    dp++;
					    DesignatorLength = 3;
					    inputSize = 3;
					    break;
					case ISO_SS3_7:
					    if (sl - sp < 3)
						return CoderResult.UNDERFLOW;
					    b2 = sa[sp +1];
					    b3 = sa[sp +1];
					    if (dl - dp <1)
						return CoderResult.OVERFLOW;
					    da[dp] = encode((byte)b2,
							        (byte)b3,
								SS3Flag);
					    dp++;
					    DesignatorLength = 3;
					    break;
					default:
					    DesignatorLength = 0;
					}
				    }
				}
			    }
			    break;
			default:
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    if (!shiftout) {
				da[dp++]=(char)sa[sp];
				inputSize = 1;
			    } else {
				if (dl - dp < 1)
				    return CoderResult.OVERFLOW;
				if (sl - sp < 2)
				    return CoderResult.UNDERFLOW;
				b2 = sa[sp+1] & 0xff;
				da[dp++] = encode((byte)b1,
					          (byte)b2,
						   SOFlag);
				inputSize = 2;
			    }
			    break;
			}
		     sp += inputSize;
		     inputSize = 0; // reset inputSize for each iteration
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
	    int DesignatorLength = 0;
	    int inputSize = 1;
	    int i;

	    try {
		while (src.hasRemaining()) {
		    b1 = src.get();
		    switch (b1) {
			case ISO_SO:
			    shiftout = true;
			    inputSize = 1;
			    break;
			case ISO_SI:
			    shiftout = false;
			    inputSize = 1;
			    break;
			case ISO_ESC:
			    int size =
				Math.min(maxDesignatorLength, 
					 src.remaining());
			    byte[] tmpBuf = new byte[size];

			    if (src.remaining() < (size + 1))
				return CoderResult.UNDERFLOW;

			    for (int o = 0 ; o < size; o++) {
				tmpBuf[o] = src.get();
			    } 

			    String tmpString =
				    new String(tmpBuf, 0, size);
			    for (i = 0; i < SODesig.length; i++) {
				if(tmpString.indexOf(SODesig[i])
							== 0) {
				    curSODes = i;
				    DesignatorLength =
					    SODesig[i].length();
			            inputSize = DesignatorLength + 1;
				    break;
				}
			    }

			    if (i == SODesig.length) {
				for (i = 0; i < SS2Desig.length; i++) {
				    if (tmpString.indexOf(SS2Desig[i]) == 0) {
					curSS2Des = i;
					DesignatorLength = SS2Desig[i].length();
					break;
				    }
				}
				if(i == SS2Desig.length) {
				    for(i = 0; i < SS3Desig.length; i++) {
					if (tmpString.indexOf(SS3Desig[i]) == 0) {
					    curSS3Des = i;
					    DesignatorLength = SS3Desig[i].length();
					    break;
					}
				    }
				    if (i == SS3Desig.length) {
					switch(b1) {
					case ISO_SS2_7:
					    if (src.remaining() < 3)
						return CoderResult.UNDERFLOW;
					    b2 = src.get();
					    b3 = src.get();
					    if (dst.remaining() < 1)
						return CoderResult.OVERFLOW;
					    dst.put(encode((byte)b2,
								(byte)b3,
								SS2Flag));
					    DesignatorLength = 3;
					    inputSize = 3;
					    break;
					case ISO_SS3_7:
					    if (src.remaining() < 2)
						return CoderResult.UNDERFLOW;
					    b2 = src.get();
					    b3 = src.get();
					    if (dst.remaining() < 1)
						return CoderResult.OVERFLOW;
					    dst.put(encode((byte)b2,
							        (byte)b3,
								SS3Flag));
					    DesignatorLength = 3;
					    break;
					default:
					    DesignatorLength = 0;
					}
				    }
				}
			    }
			    break;
			default:
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    if (!shiftout) {
				dst.put((char)b1);
				inputSize = 1;
			    } else {
				if (dst.remaining() < 1)
				    return CoderResult.OVERFLOW;
				if (src.remaining() < 1)
				    return CoderResult.UNDERFLOW;
				b2 = src.get() & 0xff;
				dst.put(encode((byte)b1,
						      (byte)b2,
							SOFlag));
				inputSize = 2;
			    }
			    break;
			}
			mark += inputSize;
			inputSize = 0;  // reset inputSize for each iteration
		    }
		    return CoderResult.UNDERFLOW;
		} catch (Exception e) { e.printStackTrace(); return CoderResult.OVERFLOW; }
		finally { 
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

    protected static class Encoder extends CharsetEncoder {


	private final byte SS2 = (byte)0x8e;
	private final byte P2 = (byte)0xA2;
	private final byte P3 = (byte)0xA3;
	private final byte MSB = (byte)0x80;

	protected final byte maximumDesignatorLength = 4;

	protected String SODesig,
			 SS2Desig = null,
			 SS3Desig = null;

	protected CharsetEncoder ISOEncoder;

	private boolean shiftout = false;
	private boolean SODesDefined = false;
	private boolean SS2DesDefined = false;
	private boolean SS3DesDefined = false;

	private boolean newshiftout = false;
	private boolean newSODesDefined = false;
	private boolean newSS2DesDefined = false;
	private boolean newSS3DesDefined = false;

	protected Encoder(Charset cs) {
	    super(cs, 4.0f, 8.0f);
	}

	public boolean canEncode(char c) {
	    return (ISOEncoder.canEncode(c));
	}

	protected void implReset() {
	    shiftout = false;
	    SODesDefined = false;
	    SS2DesDefined = false;
	    SS3DesDefined = false;
	}

	private int unicodeToNative(char unicode, byte ebyte[])
	{
	    int	index = 0;
	    byte	tmpByte[];
	    char	convChar[] = {unicode};
	    byte	convByte[] = new byte[4];
	    int 	converted;

	    try{
		CharBuffer cc = CharBuffer.wrap(convChar);
		ByteBuffer bb = ByteBuffer.allocate(4);
		ISOEncoder.encode(cc, bb, true);
		bb.flip();
		converted = bb.remaining();
		bb.get(convByte,0,converted);
	    } catch(Exception e) {
		return -1;
	    }

	    if (converted == 2) {
		if (!SODesDefined) {
		    newSODesDefined = true;
		    ebyte[0] = ISO_ESC;
		    tmpByte = SODesig.getBytes();
		    System.arraycopy(tmpByte,0,ebyte,1,tmpByte.length);
		    index = tmpByte.length+1;
		}
		if (!shiftout) {
		    newshiftout = true;
		    ebyte[index++] = ISO_SO;
		}
		ebyte[index++] = (byte)(convByte[0] & 0x7f);
		ebyte[index++] = (byte)(convByte[1] & 0x7f);
	    } else {
		if((convByte[0] == SS2) && (convByte[1] == P2)) {
		    if (!SS2DesDefined) {
			newSS2DesDefined = true;
			ebyte[0] = ISO_ESC;
			tmpByte = SS2Desig.getBytes();
			System.arraycopy(tmpByte, 0, ebyte, 1, tmpByte.length);
			index = tmpByte.length+1;
		    }
		    ebyte[index++] = ISO_ESC;
		    ebyte[index++] = ISO_SS2_7;
		    ebyte[index++] = (byte)(convByte[2] & 0x7f);
		    ebyte[index++] = (byte)(convByte[3] & 0x7f);
		}
		if((convByte[0] == SS2)&&(convByte[1] == 0xA3))
		{
		    if(!SS3DesDefined){
			newSS3DesDefined = true;
			ebyte[0] = ISO_ESC;
			tmpByte = SS3Desig.getBytes();
			System.arraycopy(tmpByte, 0, ebyte, 1, tmpByte.length);
			index = tmpByte.length+1;
		    }
		    ebyte[index++] = ISO_ESC;
		    ebyte[index++] = ISO_SS3_7;
		    ebyte[index++] = (byte)(convByte[2] & 0x7f);
		    ebyte[index++] = (byte)(convByte[3] & 0x7f);
		}
	    }
	    return index;
	}

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
	    byte[]  outputByte;
	    int     inputSize = 0;      	   // Size of input
	    byte[]  tmpBuf = new byte[8];
	    newshiftout = shiftout;
	    newSODesDefined = SODesDefined;
	    newSS2DesDefined = SS2DesDefined;
	    newSS3DesDefined = SS3DesDefined;

	    try {
		while (sp < sl) {
		    outputByte = tmpBuf;
		    if (sa[sp] < 0x80) {	// ASCII
			if (shiftout){
			    newshiftout = false;
			    outputSize = 2;
			    outputByte[0] = ISO_SI;
			    outputByte[1] = (byte)(sa[sp] & 0x7f);
			} else {
			    outputSize = 1;
			    outputByte[0] = (byte)(sa[sp] & 0x7f);
			}
			if(sa[sp] == '\n'){
			    newSODesDefined = false;
			    newSS2DesDefined = false;
			    newSS3DesDefined = false;
			}
		    } else {
			outputSize = unicodeToNative(sa[sp], outputByte);
		    }

		    if (dl - dp < outputSize)
			return CoderResult.OVERFLOW;

		    for (int i = 0; i < outputSize; i++)
			da[dp++] = outputByte[i];
		    sp++;
		    shiftout = newshiftout;
		    SODesDefined = newSODesDefined;
		    SS2DesDefined = newSS2DesDefined;
		    SS3DesDefined = newSS3DesDefined;
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
	    int outputSize = 0;
	    byte[]  outputByte;
	    int     inputSize = 0;      	   // Size of input
	    byte[]  tmpBuf = new byte[8];
	    newshiftout = shiftout;
	    newSODesDefined = SODesDefined;
	    newSS2DesDefined = SS2DesDefined;
	    newSS3DesDefined = SS3DesDefined;
	    int mark = src.position();

	    try {
		while (src.hasRemaining()) {
		    outputByte = tmpBuf;
		    char inputChar = src.get();
		    if (inputChar < 0x80) {	// ASCII
			if (shiftout){
			    newshiftout = false;
			    outputSize = 2;
			    outputByte[0] = ISO_SI;
			    outputByte[1] = (byte)(inputChar & 0x7f);
			} else {
			    outputSize = 1;
			    outputByte[0] = (byte)(inputChar & 0x7f);
			}
			if(inputChar == '\n'){
			    newSODesDefined = false;
			    newSS2DesDefined = false;
			    newSS3DesDefined = false;
			}
		    } else {
			outputSize = unicodeToNative(inputChar, outputByte);
		    }

		    if (dst.remaining() < outputSize)
			return CoderResult.OVERFLOW;
		    for (int i = 0; i < outputSize; i++)
			dst.put(outputByte[i]);
		    mark++;
		    shiftout = newshiftout;
		    SODesDefined = newSODesDefined;
		    SS2DesDefined = newSS2DesDefined;
		    SS3DesDefined = newSS3DesDefined;
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
