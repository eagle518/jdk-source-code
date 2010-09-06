/*
 * @(#)ByteToCharEUC_CN.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.EUC_CN;

/**
 * Tables and data to convert EUC_CN to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharEUC_CN extends ByteToCharDoubleByte {

    private EUC_CN nioCoder = new EUC_CN();

    public String getCharacterEncoding() {
        return "EUC_CN";
    }

    public ByteToCharEUC_CN() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0xA1;
        end = 0xFE;
    }
}
