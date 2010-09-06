/*
 * @(#)ByteToCharGB18030DB.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.GB18030;


/**
 * Tables and data to convert the two-byte portion of GB18030 to Unicode
 * The class is package level accessibility as it is just used for the
 * convenience of the main GB18030 converter.
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.4
 */

abstract class ByteToCharGB18030DB extends ByteToCharDoubleByte {

    public String getCharacterEncoding() {
        return "ByteToCharGB18030DB";
    }

    public ByteToCharGB18030DB() {
	GB18030 nioCoder = new GB18030();
        super.index1 = nioCoder.getSubDecoderIndex1();
        super.index2 = nioCoder.getSubDecoderIndex2();
        start = 0x40;
        end = 0xFE;
    }
}
