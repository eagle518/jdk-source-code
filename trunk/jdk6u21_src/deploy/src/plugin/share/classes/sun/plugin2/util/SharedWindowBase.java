/*
 * @(#)SharedWindowBase.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.lang.reflect.*;

class SharedWindowBase {
    protected static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    // Base class with reflective helper

    static {
        try {
            sun.plugin2.util.NativeLibLoader.load(new String[] {"SharedWindow"});
        } catch (Throwable t) {
            if (DEBUG) {
                // This native library is only temporary -- eventually
                // the functionality will be in the core JDK
                t.printStackTrace();
            }
        }
    }

    protected static Object invoke(Method m, Object receiver, Object[] args) {
        if (m == null) {
            throw new RuntimeException("Did not initialize successfully");
        }
        try {
            return m.invoke(receiver, args);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}
