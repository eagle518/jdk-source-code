/*
 * @(#)ConsoleController14.java	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.util.logging.Logger;

/** This interface adds additional functionality to the
    ConsoleController when we are running on top of JRE 1.4 or
    later. */

public interface ConsoleController14 extends ConsoleController {
    /** Sets the logger in use. */
    public void setLogger(Logger logger);

    /** Gets the logger in use. */
    public Logger getLogger();
}
