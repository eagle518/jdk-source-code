/*
 * @(#)ByteToCharMacSymbol.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacSymbol;

/**
 * A table to convert to MacSymbol to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacSymbol extends ByteToCharSingleByte {

    private final static MacSymbol nioCoder = new MacSymbol();

    public String getCharacterEncoding() {
        return "MacSymbol";
    }

    public ByteToCharMacSymbol() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
