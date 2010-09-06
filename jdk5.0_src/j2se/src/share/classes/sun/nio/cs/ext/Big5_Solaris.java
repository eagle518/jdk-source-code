/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Big5_Solaris.java	1.2	04/04/28
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.nio.cs.HistoricallyNamedCharset;

public class Big5_Solaris extends Charset implements HistoricallyNamedCharset
{
    public Big5_Solaris() {
	super("x-Big5-Solaris", ExtendedCharsets.aliasesFor("x-Big5-Solaris"));
    }

    public String historicalName() {
	return "Big5_Solaris";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof Big5)
		|| (cs instanceof Big5_Solaris));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends Big5.Decoder {

	protected char decodeDouble(int byte1, int byte2) {
	    char c = super.decodeDouble(byte1, byte2);

	    // Big5 Solaris implementation has 7 additional mappings

	    if (c == REPLACE_CHAR) {
		if (byte1 == 0xf9) {
		    switch (byte2) {
			case 0xD6:
			    c = (char)0x7881;
			    break;
			case 0xD7:
			    c = (char)0x92B9;
			    break;
			case 0xD8:
			    c = (char)0x88CF;
			    break;
			case 0xD9:
			    c = (char)0x58BB;
			    break;
			case 0xDA:
			    c = (char)0x6052;
			    break;
			case 0xDB:
			    c = (char)0x7CA7;
			    break;
			case 0xDC:
			    c = (char)0x5AFA;
			    break;
		    }
		} 
	    }
	    return c;
	}

	private Decoder(Charset cs) {
	    super(cs);
	}
    }

    private static class Encoder extends Big5.Encoder {

	protected int encodeDouble(char ch) {
	    int r = super.encodeDouble(ch);

	    if (r == 0) {
		switch (ch) {
		    case 0x7881:
			r = 0xF9D6;
			break;
		    case 0x92B9:
			r = 0xF9D7;
			break;
		    case 0x88CF:
			r = 0xF9D8;
			break;
		    case 0x58BB:
			r = 0xF9D9;
			break;
		    case 0x6052:
			r = 0xF9DA;
			break;
		    case 0x7CA7:
			r = 0xF9DB;
			break;
		    case 0x5AFA:
			r = 0xF9DC;
			break;
		    }
		}
	     return r;
	}

	private Encoder(Charset cs) {
	    super(cs);
	}

	public boolean canEncode(char c) {
	    int r = encodeDouble(c);
	    if (r == 0) // can't map the char
		return false;
	    else
		return true;
	}
    }
}
