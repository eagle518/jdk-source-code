/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm.sparc;

public interface SPARCV9PrivilegedRegisters {
    public static final int TPC         = 0;
    public static final int TNPC        = 1;
    public static final int TSTATE      = 2;
    public static final int TT          = 3;
    public static final int TICK        = 4;
    public static final int TBA         = 5;
    public static final int PSTATE      = 6;
    public static final int TL          = 7;
    public static final int PIL         = 8;
    public static final int CWP         = 9;
    public static final int CANSAVE     = 10;
    public static final int CANRESTORE  = 11;
    public static final int CLEANWIN    = 12;
    public static final int OTHERWIN    = 13;
    public static final int WSTATE      = 14;
    public static final int FQ          = 15;
    public static final int VER         = 31;
}
