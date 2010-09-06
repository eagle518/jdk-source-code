/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Big5_HKSCS.java	1.6 04/04/26
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.nio.cs.HistoricallyNamedCharset;

public class Big5_HKSCS extends Charset implements HistoricallyNamedCharset
{
    public Big5_HKSCS() {
	super("Big5-HKSCS", ExtendedCharsets.aliasesFor("Big5-HKSCS"));
    }

    public String historicalName() {
	return "Big5_HKSCS";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof Big5)
		|| (cs instanceof Big5_HKSCS));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends HKSCS_2001.Decoder {

	Big5.Decoder big5Dec;

	protected char decodeDouble(int byte1, int byte2) {
	    char c = super.decodeDouble(byte1, byte2);
	    return (c != REPLACE_CHAR) ? c : big5Dec.decodeDouble(byte1, byte2); 
	}

	private Decoder(Charset cs) {
	    super(cs);
	    big5Dec = new Big5.Decoder(cs);
	}
    }

    private static class Encoder extends HKSCS_2001.Encoder {

	private Big5.Encoder big5Enc; 

	protected int encodeDouble(char ch) {
	    int r = super.encodeDouble(ch);
	    return (r != 0) ? r : big5Enc.encodeDouble(ch);
	}

	private Encoder(Charset cs) {
	    super(cs);
	    big5Enc = new Big5.Encoder(cs);
	}

	public boolean canEncode(char c) {
	    int r = super.encodeDouble(c);
	    if (r == 0)
		r = big5Enc.encodeDouble(c);
	    if (r == 0) // can't map the char
		return false;
	    else
		return true;
	}
    }
}
