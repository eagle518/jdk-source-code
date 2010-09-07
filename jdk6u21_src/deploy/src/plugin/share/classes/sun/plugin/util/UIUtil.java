/*
 * @(#)UIUtil.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Component;
import java.awt.Toolkit;
import java.lang.reflect.*;
import java.security.*;

public class UIUtil {
    private UIUtil() {}

    public static void disableBackgroundErase(final Component component) {
        AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    try {
                        // Call SunToolkit.disableBackgroundErase(Component) reflectively
                        // to not have compile-time dependencies on the absolute latest 6u10
                        Toolkit toolkit = Toolkit.getDefaultToolkit();
                        Method m = toolkit.getClass().getMethod("disableBackgroundErase",
                                                                new Class[] { Component.class });
                        if (m != null) {
                            m.invoke(toolkit, new Object[] { component });
                        }
                    } catch (Exception e) {
                    } catch (Error err) {
                    }
                    return null;
                }
            });
    }
}
