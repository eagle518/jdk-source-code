/*
 * @(#)SharedWindowHost.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.lang.reflect.*;

public class SharedWindowHost extends SharedWindowBase {
    // Do this stuff reflectively to avoid compile-time dependencies
    private static Method getConnectionHandleMethod;
    private static Method linkSharedWindowToMethod;
    private static Method unlinkSharedWindowFrameMethod;

    static {
        try {
            Class c = Class.forName("apple.awt.SharedWindowHost");
            getConnectionHandleMethod     = c.getMethod("getConnectionHandle", null);
            linkSharedWindowToMethod      = c.getMethod("linkSharedWindowTo",
                                                        new Class[] { Long.TYPE, Long.TYPE });
            unlinkSharedWindowFrameMethod = c.getMethod("unlinkSharedWindowFrame",
                                                        new Class[] { Long.TYPE, Class.forName("apple.awt.SharedWindow") });
        } catch (Throwable t) {
            if (DEBUG) {
                t.printStackTrace();
            }
        }
    }

    /** The result must be sent to the child process to authorize the
        connection between the parent and child processes */
    public static long getConnectionHandle() {
        if (getConnectionHandleMethod == null) {
            return 0;
        }
        return ((Long) invoke(getConnectionHandleMethod, null, null)).longValue();
    }

    /** Links the child process's window handle to the passed one from
        the parent process */
    public static SharedWindow linkSharedWindowTo(long parentHandle, long childHandle) {
        if (linkSharedWindowToMethod == null) {
            return null;
        }
        Object obj = invoke(linkSharedWindowToMethod, null, new Object[] { new Long(parentHandle),
                                                                           new Long(childHandle) });
        return new SharedWindow(obj);
    }

    /** This should be done before the child process calls
        SharedWindowProvider.unshareWindowHandleWith */
    public static void unlinkSharedWindowFrame(long parentHandle, SharedWindow sharedWindow) {
        if (unlinkSharedWindowFrameMethod != null) {
            invoke(unlinkSharedWindowFrameMethod, null, new Object[] { new Long(parentHandle),
                                                                       sharedWindow.getWindow() });
        }
    }
}
