/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)JIS_X_0208.java	1.6 03/12/19
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;

public class JIS_X_0208
    extends Charset
{

    public JIS_X_0208() {
	super("x-JIS0208", ExtendedCharsets.aliasesFor("x-JIS0208"));
    }

    public boolean contains(Charset cs) {
	return (cs instanceof JIS_X_0208);
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new JIS_X_0208_Encoder(this);
    }

    private static class Decoder extends JIS_X_0208_Decoder {
	protected char decodeSingle(int b) {
	    return DoubleByteDecoder.REPLACE_CHAR;
	}

	public Decoder(Charset cs) {
	    super(cs);
	}
    }
}
