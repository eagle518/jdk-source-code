/*
 * @(#)Progress.java	1.5 10/05/11
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.progress;

import sun.awt.AppContext;

public class Progress {
    //Different applets may have different custom progresses at the same time
    private static String PROGRESS_KEY = "javaws.custom.progress";

    public static boolean usingCustomProgress() {
        return getCustomProgress() != null;
    }

    public static CustomProgress getCustomProgress() {
        return (CustomProgress) AppContext.getAppContext().get(PROGRESS_KEY);
    }

    public static void setCustomProgress(CustomProgress cpl) {
        AppContext.getAppContext().put(PROGRESS_KEY, cpl);
    }

    public static void reset() {
        setCustomProgress(null);
    }
}
