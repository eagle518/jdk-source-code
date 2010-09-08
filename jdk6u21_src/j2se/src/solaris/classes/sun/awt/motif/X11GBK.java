/*
 * @(#)X11GBK.java	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharsetDecoder;
import sun.nio.cs.ext.GBK;

public class X11GBK extends Charset {
    public X11GBK () {
	super("X11GBK", null);
    }
    public CharsetEncoder newEncoder() { 
        return new Encoder(this);
    }
    public CharsetDecoder newDecoder() { 
        return new GBK.Decoder(this);
    }

    public boolean contains(Charset cs) { 
        return cs instanceof X11GBK;
    }

    private class Encoder extends GBK.Encoder {
        public Encoder(Charset cs) {
	    super(cs);
	}
        public boolean canEncode(char ch){
	    if (ch < 0x80) return false;
	    return super.canEncode(ch);
	}
    }
}
