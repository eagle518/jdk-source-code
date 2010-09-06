/*
 * @(#)BMPConstants.java	1.3 03/12/19 16:53:52
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.imageio.plugins.bmp;

public interface BMPConstants {
    // bmp versions
    static final String VERSION_2 = "BMP v. 2.x";
    static final String VERSION_3 = "BMP v. 3.x";
    static final String VERSION_3_NT = "BMP v. 3.x NT";
    static final String VERSION_4 = "BMP v. 4.x";
    static final String VERSION_5 = "BMP v. 5.x";

    // Color space types
    static final int LCS_CALIBRATED_RGB = 0;
    static final int LCS_sRGB = 1;
    static final int LCS_WINDOWS_COLOR_SPACE = 2;
    static final int PROFILE_LINKED = 3;
    static final int PROFILE_EMBEDDED = 4;

    // Compression Types
    static final int BI_RGB = 0;
    static final int BI_RLE8 = 1;
    static final int BI_RLE4 = 2;
    static final int BI_BITFIELDS = 3;
    static final int BI_JPEG = 4;
    static final int BI_PNG = 5;

    static final String[] compressionTypeNames =
        {"BI_RGB", "BI_RLE8", "BI_RLE4", "BI_BITFIELDS", "BI_JPEG", "BI_PNG"};
}