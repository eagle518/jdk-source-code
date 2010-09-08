/*
 * @(#)X11CNS11643.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.nio.CharBuffer;
import java.nio.ByteBuffer;
import java.nio.charset.*;
import sun.nio.cs.ext.EUC_TW;

public abstract class X11CNS11643 extends Charset {
    private final int plane;
    public X11CNS11643 (int plane, String name) {
	super(name, null);
        switch (plane) {
	case 1:
	    this.plane = 0; // CS1
	    break;
	case 2:
	case 3:
	    this.plane = plane;
	    break;
	default:
	    throw new IllegalArgumentException
		("Only planes 1, 2, and 3 supported");
	}
    }

    public CharsetEncoder newEncoder() { 
        return new Encoder(this, plane);
    }

    public CharsetDecoder newDecoder() { 
	return new Decoder(this, plane);
    }

    public boolean contains(Charset cs) { 
        return cs instanceof X11CNS11643;
    }

    private class Encoder extends EUC_TW.Encoder {
	private int plane;
        public Encoder(Charset cs, int plane) {
	    super(cs);
	    this.plane = plane;
	}
        public boolean canEncode(char c) {
            if (c <= 0x7F) {
                return false;
            } 
            int p = getNative(c) >> 16;
	    if (p == 1 && plane == 0 ||
		p == 2 && plane == 2 ||
		p == 3 && plane == 3)
		return true; 
	    return false;
        }

        public boolean isLegalReplacement(byte[] repl) {
            return true;
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
		    if (c >= '\uFFFE' || c <= '\u007f')
			return CoderResult.unmappableForLength(1);
                    int cns = getNative(c);
                    int p = cns >> 16;
		    if (p == 1 && plane == 0 || 
                        p == 2 && plane == 2 ||
			p == 3 && plane == 3) {
			if (dl - dp < 2)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte) ((cns  >> 8) & 0x7f);
			da[dp++] = (byte) (cns & 0x7f);
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
    }    

    private class Decoder extends EUC_TW.Decoder {
        private String table;
        protected Decoder(Charset cs, int plane) {
	    super(cs);
	    switch (plane) {
	    case 0:
		table = unicodeCNS1;
		break;
	    case 2:
		table = unicodeCNS2;
		break;
	    case 3:
		table = unicodeCNS3;
		break;
	    default:
                throw new IllegalArgumentException
		    ("Only planes 1, 2, and 3 supported");
	    }
        }

        //we only work on array backed buffer.
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
                    byte b1 = sa[sp];
                    byte b2 = sa[sp + 1];
                    char c = convToUnicode((byte)(b1 | 0x80), 
					   (byte)(b2 | 0x80), 
					   table);
		    if (c == replacement().charAt(0) 
			//to keep the compatibility with b2cX11CNS11643
			/*|| c == '\u0000'*/) {   
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
