/*
 * @(#)FontCheckerConstants.java	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * <PRE>
 * FontCheckerConstants contains constants shared between
 * FontChecker and FontCheckDummy classes. 
 *
 * </PRE>
 *
 * @author Ilya Bagrak
 */

package com.sun.java2d.fontchecker;

public interface FontCheckerConstants {

    /* code sent to indicate child process started OK */
    public static final int CHILD_STARTED_OK   = 100;
 
    /* error codes returned from child process */ 
    public static final int ERR_FONT_OK         = 65; 
    public static final int ERR_FONT_NOT_FOUND  = 60; 
    public static final int ERR_FONT_BAD_FORMAT = 61; 
    public static final int ERR_FONT_READ_EXCPT = 62; 
    public static final int ERR_FONT_DISPLAY    = 64; 
    public static final int ERR_FONT_EOS        = -1;
    /* nl char sent after child crashes */
    public static final int ERR_FONT_CRASH      = 10; 

    /* 0 and 1 are reserved, and commands can only be a single digit integer */
    public static final int EXITCOMMAND = 2;
}
