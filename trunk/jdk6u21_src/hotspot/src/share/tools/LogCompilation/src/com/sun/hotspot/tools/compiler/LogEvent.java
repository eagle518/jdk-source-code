/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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
 */

package com.sun.hotspot.tools.compiler;

import java.io.PrintStream;
import java.util.*;

public interface LogEvent {
    public double getStart();

    public double getElapsedTime();

    public Compilation getCompilation();

    public void print(PrintStream stream);
}
