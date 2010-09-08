/*
 * @(#)COMPOUND_TEXT.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharsetDecoder;

public class COMPOUND_TEXT extends Charset {
    public COMPOUND_TEXT () {
	super("COMPOUND_TEXT", null);
    }

    public CharsetEncoder newEncoder() { 
        return new COMPOUND_TEXT_Encoder(this); 
    }

    public CharsetDecoder newDecoder() { 
        return new COMPOUND_TEXT_Decoder(this); 
    }

    public boolean contains(Charset cs) { 
        return cs instanceof COMPOUND_TEXT;
    }
}
