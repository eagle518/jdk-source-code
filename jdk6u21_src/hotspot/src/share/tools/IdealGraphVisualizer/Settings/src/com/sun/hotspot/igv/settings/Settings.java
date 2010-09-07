/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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
 *
 */
package com.sun.hotspot.igv.settings;

import java.util.prefs.Preferences;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Settings {

    public final static String NODE_TEXT = "nodeText";
    public final static String NODE_TEXT_DEFAULT = "[idx] [name]";
    public final static String NODE_WIDTH = "nodeWidth";
    public final static String NODE_WIDTH_DEFAULT = "100";
    public final static String PORT = "port";
    public final static String PORT_DEFAULT = "4444";
    public final static String DIRECTORY = "directory";
    public final static String DIRECTORY_DEFAULT = System.getProperty("user.dir");

    public static Preferences get() {
        return Preferences.userNodeForPackage(Settings.class);
    }
}
