/*
 * @(#)SystemUtils.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import com.sun.deploy.config.Config;

/** System-level helper utilities. */

public class SystemUtils {
    // @return the current time in micro seconds (10 pow -6)
    public static final long microTime() {
        if(Config.isJavaVersionAtLeast15()) {
            return System.nanoTime()/1000;
        } else {
            return System.currentTimeMillis()*1000;
        }
    }
}

