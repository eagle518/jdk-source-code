/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.debugger.win32;

/** Enumerates flags in Win32LDTEntry */

interface Win32LDTEntryConstants {
  // Types of segments
  public static final int TYPE_READ_ONLY_DATA                      = 0;
  public static final int TYPE_READ_WRITE_DATA                     = 1;
  public static final int TYPE_UNUSED                              = 2;
  public static final int TYPE_READ_WRITE_EXPAND_DOWN_DATA         = 3;
  public static final int TYPE_EXECUTE_ONLY_CODE                   = 4;
  public static final int TYPE_EXECUTABLE_READABLE_CODE            = 5;
  public static final int TYPE_EXECUTE_ONLY_CONFORMING_CODE        = 6;
  public static final int TYPE_EXECUTABLE_READABLE_CONFORMING_CODE = 7;
}
