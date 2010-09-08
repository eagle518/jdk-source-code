/*
 * @(#)X11GB2312.java	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.nio.CharBuffer;
import java.nio.ByteBuffer;
import java.nio.charset.*;
import sun.nio.cs.ext.EUC_CN;

public class X11GB2312 extends Charset {
    public X11GB2312 () {
	super("X11GB2312", null);
    }
    public CharsetEncoder newEncoder() { 
        return new Encoder(this);
    }
    public CharsetDecoder newDecoder() { 
	return new Decoder(this);
    }

    public boolean contains(Charset cs) { 
        return cs instanceof X11GB2312;
    }

    private class Encoder extends EUC_CN.Encoder {
        public Encoder(Charset cs) {
	    super(cs);
	}

	public boolean canEncode(char c) {
            if (c <= 0x7F) {
                return false;
            } 
	    return super.canEncode(c);
        }

        protected CoderResult encodeLoop(CharBuffer src, ByteBuffer dst) {
	    char[] sa = src.array();
	    int sp = src.arrayOffset() + src.position();
	    int sl = src.arrayOffset() + src.limit();

 	    byte[] da = dst.array();
	    int dp = dst.arrayOffset() + dst.position();
	    int dl = dst.arrayOffset() + dst.limit();
    
	    try {
		while (sp < sl) {
		    char c = sa[sp];
		    if (c <= '\u007f')
			return CoderResult.unmappableForLength(1);
                    int ncode = encodeDouble(c);
                    if (ncode != 0 && c != '\u0000' ) {
			da[dp++] = (byte) ((ncode  >> 8) & 0x7f);
			da[dp++] = (byte) (ncode & 0x7f);
			sp++;
			continue;
		    }
		    return CoderResult.unmappableForLength(1);
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }
        }
        public boolean isLegalReplacement(byte[] repl) {
            return true;
        }
    }

    private class Decoder extends EUC_CN.Decoder {
        public Decoder(Charset cs) {
	    super(cs);
	}

        protected CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
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
		    if ( sl - sp < 2) {
			return CoderResult.UNDERFLOW;
		    }
                    int b1 = sa[sp] & 0xFF | 0x80;
                    int b2 = sa[sp + 1] & 0xFF | 0x80;
                    char c = decodeDouble(b1, b2);
		    if (c == replacement().charAt(0)) {
			return CoderResult.unmappableForLength(2);
		    }
		    if (dl - dp < 1)
			return CoderResult.OVERFLOW;
		    da[dp++] = c;
		    sp +=2;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }

        }
    }

}
