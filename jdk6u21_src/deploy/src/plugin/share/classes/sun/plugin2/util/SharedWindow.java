/*
 * @(#)SharedWindow.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.lang.reflect.*;

public class SharedWindow extends SharedWindowBase {
    // Do this stuff reflectively to avoid compile-time dependencies
    private static Method setVisibleMethod;
    private static Method setLocationMethod;
    private static Method setClipMethod;
    
    static {
        try {
            Class c = Class.forName("apple.awt.SharedWindow");
            setVisibleMethod = c.getMethod("setVisible", new Class[] { Boolean.TYPE });
            setLocationMethod = c.getMethod("setLocation", new Class[] { Double.TYPE, Double.TYPE });
            setClipMethod = c.getMethod("setClip", new Class[] { Double.TYPE, Double.TYPE, Double.TYPE, Double.TYPE });
        } catch (Throwable t) {
            if (DEBUG) {
                t.printStackTrace();
            }
        }
    }

    private Object obj;

    public SharedWindow(Object obj) {
        this.obj = obj;
    }

    public void setVisible(boolean visible) {
        if (setVisibleMethod != null) {
            invoke(setVisibleMethod, obj, new Object[] { Boolean.valueOf(visible) });
        }
    }

    public void setLocation(double x, double y) {
        if (setLocationMethod != null) {
            invoke(setLocationMethod, obj, new Object[] { new Double(x), new Double(y) });
        }
    }

    public void setClip(double x, double y, double width, double height) {
        if (setClipMethod != null) {
            invoke(setClipMethod, obj, new Object[] { new Double(x), new Double(y),
                                                      new Double(width), new Double(height) });
        }
    }

    public Object getWindow() {
        return obj;
    }
}
