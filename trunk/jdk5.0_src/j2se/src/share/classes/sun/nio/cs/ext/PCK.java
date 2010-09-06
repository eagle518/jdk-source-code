/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PCK.java	1.5	03/12/19
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import sun.nio.cs.HistoricallyNamedCharset;

public class PCK
    extends Charset
    implements HistoricallyNamedCharset
{

    public PCK() {
	super("x-PCK", ExtendedCharsets.aliasesFor("x-PCK"));
    }

    public String historicalName() {
	return "PCK";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof JIS_X_0201)
		|| (cs instanceof PCK));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {

	// Need to force the replacement byte to 0x3f 
	// because JIS_X_0208_Encoder defines its own
	// alternative 2 byte substitution to permit it
	// to exist as a self-standing Encoder

	byte[] replacementBytes = { (byte)0x3f };
	return new Encoder(this).replaceWith(replacementBytes);
    }

    private static class Decoder extends SJIS.Decoder {

	JIS_X_0208_Solaris_Decoder jis0208;
	private static final char REPLACE_CHAR='\uFFFD';

	private Decoder(Charset cs) {
	    super(cs);
	    jis0208 = new JIS_X_0208_Solaris_Decoder(cs);
	}

	protected char decodeDouble(int c1, int c2) {
	    char outChar;

	    if ((outChar = super.decodeDouble(c1, c2)) != '\uFFFD')  {
	        // Map JIS X 0208:1983 0x213D <--> U+2015 
		return ((outChar != '\u2014')? outChar: '\u2015');
	    } else {
		int adjust = c2 < 0x9F ? 1 : 0;
		int rowOffset = c1 < 0xA0 ? 0x70 : 0xB0;
		int cellOffset = (adjust == 1) ? (c2 > 0x7F ? 0x20 : 0x1F) : 0x7E;
		int b1 = ((c1 - rowOffset) << 1) - adjust;
		int b2 = c2 - cellOffset;
		char outChar2 = jis0208.decodeDouble(b1, b2);
		return outChar2;
	    }
	}
    }

    private static class Encoder extends SJIS.Encoder {

	private JIS_X_0201.Encoder jis0201; 
	private JIS_X_0208_Solaris_Encoder jis0208;

	short[] j0208Index1;	
	String[] j0208Index2;	

	private Encoder(Charset cs) {
	    super(cs);
	    jis0201 = new JIS_X_0201.Encoder(cs);
	    jis0208 = new JIS_X_0208_Solaris_Encoder(cs);
	    j0208Index1 = jis0208.getIndex1();
	    j0208Index2 = jis0208.getIndex2();
	}

	public boolean canEncode(char c) {
	    return (c < '\u00FF'
		    || (encodeDouble(c) != 0)) ;
	}

	protected int encodeDouble(char ch) {
	    int result = 0;

	    // PCK uses JIS_X_0208:1983 rather than JIS_X_0208:1997

	    switch (ch) {
		case '\u2015':
		    return (int)0x815C;
		case '\u2014':
		    return 0;
		default:
		    break;
	    }

	    if ((result = super.encodeDouble(ch)) != 0) {
		return result;
	    }
	    else {
		int offset = j0208Index1[ch >> 8] << 8;
		int pos = j0208Index2[offset >> 12].charAt((offset & 0xfff) + (ch & 0xff));
		if (pos != 0) {
		int c1 = (pos >> 8) & 0xff;
		int c2 = pos & 0xff;
		int rowOffset = c1 < 0x5F ? 0x70 : 0xB0;
		int cellOffset = (c1 % 2 == 1) ? (c2 > 0x5F ? 0x20 : 0x1F) : 0x7E;
		result = ((((c1 + 1 ) >> 1) + rowOffset) << 8) | (c2 + cellOffset);
		}
	    }
	    return result;
	}
    }
}
