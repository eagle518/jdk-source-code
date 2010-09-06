/*
 * @(#)CharToByteMS932DB.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MS932DB;

/**
 * Tables and data to convert Unicode to MS932
 *
 * @author  ConverterGenerator tool
 */

abstract class CharToByteMS932DB extends CharToByteDoubleByte {

    public CharToByteMS932DB() {
        super.index1 = MS932DB.Encoder.index1;
        super.index2 = MS932DB.Encoder.index2;
    }
}

