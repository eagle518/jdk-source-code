/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MS950_HKSCS.java	1.6 04/04/26
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.nio.cs.HistoricallyNamedCharset;

public class MS950_HKSCS extends Charset implements HistoricallyNamedCharset
{
    public MS950_HKSCS() {
	super("x-MS950-HKSCS", ExtendedCharsets.aliasesFor("x-MS950-HKSCS"));
    }

    public String historicalName() {
	return "MS950_HKSCS";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS950)
		|| (cs instanceof MS950_HKSCS));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends HKSCS.Decoder {

	private MS950.Decoder ms950Dec;


	/*
	 * Note current decoder decodes 0x8BC2 --> U+F53A
	 * ie. maps to Unicode PUA.
	 * Unaccounted discrepancy between this mapping
	 * inferred from MS950/windows-950 and the published
	 * MS HKSCS mappings which maps 0x8BC2 --> U+5C22 
	 * a character defined with the Unified CJK block
	 */

	protected char decodeDouble(int byte1, int byte2) {
	    char c = super.decodeDouble(byte1, byte2);
	    return (c != REPLACE_CHAR) ? c : ms950Dec.decodeDouble(byte1, byte2); 
	}

	private Decoder(Charset cs) {
	    super(cs);
	    ms950Dec = new MS950.Decoder(cs);
	}
    }

    private static class Encoder extends HKSCS.Encoder {

	private MS950.Encoder ms950Enc; 

	/*
	 * Note current encoder encodes U+F53A --> 0x8BC2
	 * Published MS HKSCS mappings show 
	 * U+5C22 <--> 0x8BC2
	 */
	protected int encodeDouble(char ch) {
	    int r = super.encodeDouble(ch);
	    return (r != 0) ? r : ms950Enc.encodeDouble(ch);
	}

	private Encoder(Charset cs) {
	    super(cs);
	    ms950Enc = new MS950.Encoder(cs);
	}

	public boolean canEncode(char c) {
	    int r = super.encodeDouble(c);
	    if (r == 0)
		r = ms950Enc.encodeDouble(c);
	    if (r == 0) // can't map the char
		return false;
	    else
		return true;
	}
    }
}
