/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)IBM942C.java	1.4 04/04/28
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharacterCodingException;
import sun.nio.cs.HistoricallyNamedCharset;

public class IBM942C extends Charset implements HistoricallyNamedCharset
{

    public IBM942C() {
	super("x-IBM942C", ExtendedCharsets.aliasesFor("x-IBM942C"));
    }

    public String historicalName() {
	return "Cp942C";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof IBM942C));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends IBM942.Decoder {
        protected static final String singleByteToChar;

	static {
	  String indexs = "";
	  for (char c = '\0'; c < '\u0080'; ++c) indexs += c;
	      singleByteToChar = indexs +
				 IBM942.Decoder.singleByteToChar.substring(indexs.length());
	}

	public Decoder(Charset cs) {
	    super(cs, singleByteToChar);
	}
    }

    private static class Encoder extends IBM942.Encoder {

   protected static final short index1[];
   protected static final String index2a;
   protected static final int shift = 5;

	static {

	    String indexs = "";
	    for (char c = '\0'; c < '\u0080'; ++c) indexs += c;
		index2a = IBM942.Encoder.index2a + indexs;

	    int o = IBM942.Encoder.index2a.length() + 15000;
	    index1 = new short[IBM942.Encoder.index1.length];
	    System.arraycopy(IBM942.Encoder.index1, 0, index1, 0, IBM942.Encoder.index1.length);

	    for (int i = 0; i * (1<<shift) < 128; ++i) {
		index1[i] = (short)(o + i * (1<<shift));
	    }
	}

	public Encoder(Charset cs) {
	    super(cs, index1, index2a);
	}
    }
}
