/*
 * @(#)PerfRollup.java	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.io.PrintStream;
import com.sun.deploy.perf.PerfLabel;

public interface PerfRollup {
    /**
     * Perform a rollup of the given <code>PerfLabels</code>.
     */
    public void doRollup(PerfLabel [] labels, PrintStream out);
}
