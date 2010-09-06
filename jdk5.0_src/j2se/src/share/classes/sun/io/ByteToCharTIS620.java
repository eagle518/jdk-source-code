/*
 * @(#)ByteToCharTIS620.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.TIS_620;

/**
 * A table to convert TIS620 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharTIS620 extends ByteToCharSingleByte {

    private final static TIS_620 nioCoder = new TIS_620();

    public String getCharacterEncoding() {
        return "TIS620";
    }

    public ByteToCharTIS620() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
