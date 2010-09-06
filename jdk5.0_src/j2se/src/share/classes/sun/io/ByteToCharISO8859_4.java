/*
 * @(#)ByteToCharISO8859_4.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ISO_8859_4;

/**
 * A table to convert ISO8859_4 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharISO8859_4 extends ByteToCharSingleByte {

    private final static ISO_8859_4 nioCoder = new ISO_8859_4();

    public String getCharacterEncoding() {
        return "ISO8859_4";
    }

    public ByteToCharISO8859_4() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
