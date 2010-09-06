/*
 * @(#)ByteToCharKOI8_R.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.KOI8_R;

/**
 * A table to convert KOI8_R to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharKOI8_R extends ByteToCharSingleByte {

    private final static KOI8_R nioCoder = new KOI8_R();

    public String getCharacterEncoding() {
        return "KOI8_R";
    }

    public ByteToCharKOI8_R() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
