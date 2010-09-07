/*
 *  @(#)ServerPrintHelper.java	1.1 07/11/29
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.nio.*;

/** Helps abstract applet native printing code envoked by Plugin server side 
 */

public class ServerPrintHelper {
    // We expect that the native code for this class, when called, is already loaded

    private ServerPrintHelper() {}

    public static boolean isPrinterDC(long hdc) 
    {
        return isPrinterDC0(hdc);
    }

    private static native boolean isPrinterDC0(long hdc);

    public static boolean printBand(long hdc, ByteBuffer imageArray, int offset,
                                    int srcX, int srcY, int srcWidth, int srcHeight,
                                    int destX, int destY, int destWidth, int destHeight) {
        return printBand0(hdc, imageArray, offset,
                          srcX, srcY, srcWidth, srcHeight,
                          destX, destY, destWidth, destHeight);
    }

    private static native boolean printBand0(long hdc, ByteBuffer imageArray, int offset,
                                             int srcX, int srcY, int srcWidth, int srcHeight,
                                             int destX, int destY, int destWidth, int destHeight);

}
