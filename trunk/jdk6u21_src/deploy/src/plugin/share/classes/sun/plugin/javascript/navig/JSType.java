/*
 * @(#)JSType.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSException;

/** 
 * <p> sun.plugin.javascript.navig.JSObject is a base class for all
 * JSObject representation in Navigator inside Java Plug-in. e.g.
 * Window, Document, Location, ..., and so on. 
 * </p>
 */
public interface JSType {
    public static String Anchor		= "[object Anchor]";
    public static String AnchorArray	= "[object AnchorArray]";
    public static String Document	= "[object Document]";
    public static String Element	= "[object Element]";
    public static String ElementArray	= "[object ElementArray]";
    public static String Form		= "[object Form]";
    public static String FormArray	= "[object FormArray]";
    public static String FrameArray	= "[object FrameArray]";
    public static String History	= "[object History]";
    public static String Image		= "[object Image]";
    public static String ImageArray	= "[object ImageArray]";
    public static String Layer		= "[object Layer]";
    public static String LayerArray	= "[object LayerArray]";
    public static String Link		= "[object Link]";
    public static String LinkArray	= "[object LinkArray]";
    public static String Location	= "[object Location]";
    public static String Navigator	= "[object Navigator]";
    public static String Option		= "[object Option]";
    public static String OptionArray	= "[object OptionArray]";
    public static String UIBar		= "[object UIBar]";
    public static String URL		= "[object URL]";
    public static String Window		= "[object Window]";
    public static String Embed		= "[object Embed]";
    public static String EmbedArray	= "[object EmbedArray]";
    public static String Applet		= "[object Applet]";
    public static String AppletArray	= "[object AppletArray]";
}
