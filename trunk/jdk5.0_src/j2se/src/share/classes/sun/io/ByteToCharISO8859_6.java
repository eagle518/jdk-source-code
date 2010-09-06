/*
 * @(#)ByteToCharISO8859_6.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.ISO_8859_6;

/**
 * A table to convert ISO8859_6 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharISO8859_6 extends ByteToCharSingleByte {

    private final static ISO_8859_6 nioCoder = new ISO_8859_6();

    public String getCharacterEncoding() {
        return "ISO8859_6";
    }

    public ByteToCharISO8859_6() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
