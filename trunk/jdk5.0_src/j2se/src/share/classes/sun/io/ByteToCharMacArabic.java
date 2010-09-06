/*
 * @(#)ByteToCharMacArabic.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacArabic;

/**
 * A table to convert to MacArabic to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacArabic extends ByteToCharSingleByte {

    private final static MacArabic nioCoder = new MacArabic();

    public String getCharacterEncoding() {
        return "MacArabic";
    }

    public ByteToCharMacArabic() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
