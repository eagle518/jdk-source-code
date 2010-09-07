/*
 * @(#)SharedWindowAWT.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.lang.reflect.*;

public class SharedWindowAWT extends SharedWindowBase {
    // Do this stuff reflectively to avoid compile-time dependencies
    private static Method windowHandleForMethod;
    private static Method setHasShadowMethod;

    static {
        try {
            Class c = Class.forName("apple.awt.SharedWindowAWT");
            windowHandleForMethod = c.getMethod("windowHandleFor", new Class[] { java.awt.Window.class });
            setHasShadowMethod    = c.getMethod("setHasShadow", new Class[] { java.awt.Window.class, Boolean.TYPE });
        } catch (Throwable t) {
            if (DEBUG) {
                t.printStackTrace();
            }
        }
    }

    public static long windowHandleFor(java.awt.Window window) {
        if (windowHandleForMethod == null) {
            return 0;
        }
        return ((Long) invoke(windowHandleForMethod, null, new Object[] { window })).longValue();
    }

    public static void setHasShadow(java.awt.Window window, boolean hasShadow) {
        if (setHasShadowMethod != null) {
            invoke(setHasShadowMethod, null, new Object[] { Boolean.valueOf(hasShadow) });
        }
    }
}
