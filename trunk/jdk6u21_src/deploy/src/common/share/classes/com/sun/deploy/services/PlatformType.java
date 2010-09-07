/*
 * @(#)PlatformType.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;


/** 
 * PlatformType is an interface that encapsulates the platform/browser type.
 */
public final class PlatformType
{
    private final static int UNKNOWN = 0x0000;

    private final static int BROWSER_WIN32 = 0x0100;

    private final static int BROWSER_UNIX = 0x1000;

    private final static int INTERNET_EXPLORER = 0x0001;

    private final static int NETSCAPE4 = 0x0002;

    private final static int NETSCAPE6 = 0x0003;

    private final static int NETSCAPE45 = 0x0004;

    public final static int AXBRIDGE = 0x0005;

    private final static int STANDALONE_MANTIS = 0x4000;

    private final static int STANDALONE_TIGER = 0x8000;

    public final static int INTERNET_EXPLORER_WIN32 = BROWSER_WIN32 | INTERNET_EXPLORER;

    public final static int NETSCAPE4_WIN32 = BROWSER_WIN32 | NETSCAPE4;

    public final static int NETSCAPE45_WIN32 = BROWSER_WIN32 | NETSCAPE45;

    public final static int NETSCAPE6_WIN32 = BROWSER_WIN32 | NETSCAPE6;

    public final static int NETSCAPE4_UNIX = BROWSER_UNIX | NETSCAPE4;

    public final static int NETSCAPE45_UNIX = BROWSER_UNIX | NETSCAPE45;

    public final static int NETSCAPE6_UNIX = BROWSER_UNIX | NETSCAPE6;

    public final static int STANDALONE_MANTIS_WIN32 = BROWSER_WIN32 | STANDALONE_MANTIS;

    public final static int STANDALONE_MANTIS_UNIX = BROWSER_UNIX | STANDALONE_MANTIS;

    public final static int STANDALONE_TIGER_WIN32 = BROWSER_WIN32 | STANDALONE_TIGER;

    public final static int STANDALONE_TIGER_UNIX = BROWSER_UNIX | STANDALONE_TIGER;
}



