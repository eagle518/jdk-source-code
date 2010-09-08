/*
 * @(#)X11JIS0212.java	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharsetDecoder;
import sun.nio.cs.ext.JIS_X_0212_Encoder;
import sun.nio.cs.ext.JIS_X_0212_Decoder;

public class X11JIS0212 extends Charset {
    public X11JIS0212 () {
	super("X11JIS0212", null);
    }
    public CharsetEncoder newEncoder() { 
        return new JIS_X_0212_Encoder(this); 
    }
    public CharsetDecoder newDecoder() { 
        return new JIS_X_0212_Decoder(this); 
    }

    public boolean contains(Charset cs) { 
        return cs instanceof X11JIS0212;
    }
}
