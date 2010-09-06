/*
 * @(#)UTF_16BE.java	1.18	04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;
import java.nio.charset.UnmappableCharacterException;
import sun.nio.cs.UnicodeDecoder;
import sun.nio.cs.UnicodeEncoder;


class UTF_16BE
    extends Charset
    implements HistoricallyNamedCharset
{

    public UTF_16BE() {
	super("UTF-16BE", StandardCharsets.aliases_UTF_16BE);
    }

    public String historicalName() {
	return "UnicodeBigUnmarked";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof US_ASCII)
		|| (cs instanceof ISO_8859_1)
		|| (cs instanceof ISO_8859_15)
		|| (cs instanceof MS1252)
		|| (cs instanceof UTF_8)
		|| (cs instanceof UTF_16)
		|| (cs instanceof UTF_16BE)
		|| (cs instanceof UTF_16LE)
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
	        || (cs.name().equals("SJIS"))
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
	        || (cs.name().equals("Shift_JIS")));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends UnicodeDecoder {

	public Decoder(Charset cs) {
	    super(cs, BIG);
	}
    }

    private static class Encoder extends UnicodeEncoder {

	public Encoder(Charset cs) {
	   super(cs, BIG, false);
	}

	public boolean canEncode(char c) {
	    if (Surrogate.isHigh(c) || Surrogate.isLow(c))
	       return false;
	    else
	       return (c<='\uFFFF'); 
	}
    }

}

