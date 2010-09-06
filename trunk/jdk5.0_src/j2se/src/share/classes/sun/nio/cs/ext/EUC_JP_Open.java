/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)EUC_JP_Open.java	1.3 03/12/19
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

public class EUC_JP_Open
    extends Charset
    implements HistoricallyNamedCharset
{
    public EUC_JP_Open() {
	super("x-eucJP-Open", ExtendedCharsets.aliasesFor("x-eucJP-Open"));
    }

    public String historicalName() {
	return "EUC_JP_Solaris";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof JIS_X_0201)
		|| (cs instanceof EUC_JP));
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

    private static class Decoder extends EUC_JP.Decoder {
	JIS_X_0201.Decoder decoderJ0201;
	JIS_X_0212_Solaris_Decoder decodeMappingJ0212;
	JIS_X_0208_Solaris_Decoder decodeMappingJ0208;

	short[] j0208Index1;
	String[] j0208Index2;

        protected final char REPLACE_CHAR='\uFFFD';

	private Decoder(Charset cs) {
	    super(cs);
	    decoderJ0201 = new JIS_X_0201.Decoder(cs);
	    decodeMappingJ0212 = new JIS_X_0212_Solaris_Decoder(cs);
	    decodeMappingJ0208 = new JIS_X_0208_Solaris_Decoder(cs);
	    decodeMappingJ0208.start = 0xa1;
	    decodeMappingJ0208.end = 0xfe;
	    j0208Index1 = decodeMappingJ0208.getIndex1();
	    j0208Index2 = decodeMappingJ0208.getIndex2();
	}


	protected char decode0212(int byte1, int byte2) {
	     return decodeMappingJ0212.decodeDouble(byte1, byte2);
	    
        }

	public boolean canEncode(char ch) {
	    return true;
	}

	protected char decodeDouble(int byte1, int byte2) {
	    if (byte1 == 0x8e) {
		return decoderJ0201.decode(byte2 - 256);
	    }

	    if (((byte1 < 0)
	        || (byte1 > decodeMappingJ0208.getIndex1().length))
		|| ((byte2 < decodeMappingJ0208.start)
		|| (byte2 > decodeMappingJ0208.end)))
		return REPLACE_CHAR;

	    char result = super.decodeDouble(byte1, byte2);
	    if (result != '\uFFFD') {
		return result;
	    } else {
		int n = (j0208Index1[byte1 - 0x80] & 0xf) *
			(decodeMappingJ0208.end - decodeMappingJ0208.start + 1)
			+ (byte2 - decodeMappingJ0208.start);
		return j0208Index2[j0208Index1[byte1 - 0x80] >> 4].charAt(n);
	    }
	}
    }


    private static class Encoder extends EUC_JP.Encoder {

	JIS_X_0201.Encoder encoderJ0201;
	JIS_X_0212_Solaris_Encoder encoderJ0212;
	JIS_X_0208_Solaris_Encoder encoderJ0208;
	
	short[] j0208Index1;
	String[] j0208Index2;

	private final Surrogate.Parser sgp = new Surrogate.Parser();

	private Encoder(Charset cs) {
	    super(cs);
	    encoderJ0201 = new JIS_X_0201.Encoder(cs);
	    encoderJ0212 = new JIS_X_0212_Solaris_Encoder(cs);
	    encoderJ0208 = new JIS_X_0208_Solaris_Encoder(cs);
	    j0208Index1 = encoderJ0208.getIndex1();
	    j0208Index2 = encoderJ0208.getIndex2();
	}

	protected int encodeSingle(char inputChar, byte[] outputByte) {
	    byte b;

	    if (inputChar == 0) {
		outputByte[0] = (byte)0;
		return 1;
	    }

	    if ((b = encoderJ0201.encode(inputChar)) == 0)
		return 0;

	    if (b > 0 && b < 128) {
		outputByte[0] = b;
		return 1;
	    }

	    outputByte[0] = (byte)0x8e;
	    outputByte[1] = b;
	    return 2;
	}

	protected int encodeDouble(char ch) {
	    int r = super.encodeDouble(ch);
	    if (r != 0) {
		return r;
	    }
	    else {
		int offset = j0208Index1[((ch & 0xff00) >> 8 )] << 8;
		r = j0208Index2[offset >> 12].charAt((offset & 0xfff) +
		    (ch & 0xFF));
		if (r > 0x7500)
		   return 0x8F8080 + encoderJ0212.encodeDouble(ch);
		}
	        return (r==0 ? 0: r + 0x8080);
	}
    }
}
