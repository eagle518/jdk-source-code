/*
 * @(#)CharToByteEUC_CN.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.EUC_CN;

/**
 * Tables and data to convert Unicode to EUC_CN
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteEUC_CN extends CharToByteDoubleByte {

    private final static EUC_CN nioCoder = new EUC_CN();

    public String getCharacterEncoding() {
        return "EUC_CN";
    }

    public CharToByteEUC_CN() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
