/*
 * @(#)TraceFilter.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;


/**
 * TraceFilter is an interface which contains all the possible trace
 * filters.
 */
public interface TraceFilter 
{
    /**
     * OUTPUT_MASK
     */
    public final int OUTPUT_MASK = 0x000F;

    /**
     * STATUS_BAR. Trace message can be output to browser's status bar.
     */
    public final int STATUS_BAR_ONLY = 0x0001;


    /**
     * JAVA_CONSOLE. Trace message can be output to Java Console.
     */
    public final int JAVA_CONSOLE_ONLY = 0x0002;


    /**
     * DEFAULT. Trace message can be output to the default display.
     */
    public final int DEFAULT = JAVA_CONSOLE_ONLY;

    /**
     * LEVEL_MASK
     */
    public final int LEVEL_MASK = 0x0FF0;

    /**
     * LEVEL_BASIC
     */
    public final int LEVEL_BASIC = 0x0010;

    /**
     * LEVEL_NET
     */
    public final int LEVEL_NET = 0x0020;

    /**
     * LEVEL_SECURITY
     */
    public final int LEVEL_SECURITY = 0x0040;

    /**
     * LEVEL_EXT
     */
    public final int LEVEL_EXT = 0x0080;

    /**
     * LEVEL_LIVECONNECT
     */
    public final int LEVEL_LIVECONNECT = 0x0100;
}

