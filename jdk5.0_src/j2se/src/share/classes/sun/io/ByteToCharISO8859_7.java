/*
 * @(#)ByteToCharISO8859_7.java	1.18 04/07/27
 *
 * Copyright (c) 2004 Sun Microsystems, Inc.,
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ISO_8859_7;


/**
 * A table to convert ISO8859_7 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharISO8859_7 extends ByteToCharSingleByte {

    private final static ISO_8859_7 nioCoder = new ISO_8859_7();

    public String getCharacterEncoding() {
        return "ISO8859_7";
    }

    public ByteToCharISO8859_7() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
