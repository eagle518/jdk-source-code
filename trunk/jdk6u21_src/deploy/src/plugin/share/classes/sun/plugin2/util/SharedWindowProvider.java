/*
 * @(#)SharedWindowProvider.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.lang.reflect.*;

public class SharedWindowProvider extends SharedWindowBase {
    // Do this stuff reflectively to avoid compile-time dependencies
    private static Method authorizeParentMethod;
    private static Method deauthorizeParentMethod;
    private static Method shareWindowHandleMethod;
    private static Method unshareWindowHandleMethod;

    static {
        try {
            Class c = Class.forName("apple.awt.SharedWindowProvider");
            authorizeParentMethod     = c.getMethod("authorizeParent", new Class[] { Long.TYPE });
            deauthorizeParentMethod   = c.getMethod("deauthorizeParent", new Class[] { Long.TYPE });
            shareWindowHandleMethod   = c.getMethod("shareWindowHandle", new Class[] { Long.TYPE });
            unshareWindowHandleMethod = c.getMethod("unshareWindowHandle", new Class[] { Long.TYPE });
        } catch (Throwable t) {
            if (DEBUG) {
                t.printStackTrace();
            }
        }
    }

    public static boolean initializedSuccessfully() {
        return (authorizeParentMethod != null &&
                deauthorizeParentMethod != null &&
                shareWindowHandleMethod != null &&
                unshareWindowHandleMethod != null);
    }

    public static void authorizeParent(long parentHandle) {
        if (authorizeParentMethod != null) {
            invoke(authorizeParentMethod, null, new Object[] { new Long(parentHandle) });
        }
    }

    public static void deauthorizeParent(long parentHandle) {
        if (deauthorizeParentMethod != null) {
            invoke(deauthorizeParentMethod, null, new Object[] { new Long(parentHandle) });
        }
    }

    /** This must be called with the result of e.g.
        SharedWindowAWT.windowHandleFor(Window) before calling
        SharedWindowHost.linkSharedWindowTo() on the server side */
    public static void shareWindowHandle(long windowNumber) {
        if (shareWindowHandleMethod != null) {
            invoke(shareWindowHandleMethod, null, new Object[] { new Long(windowNumber) });
        }
    }

    public static void unshareWindowHandle(long windowNumber) {
        if (unshareWindowHandleMethod != null) {
            invoke(unshareWindowHandleMethod, null, new Object[] { new Long(windowNumber) });
        }
    }
}
