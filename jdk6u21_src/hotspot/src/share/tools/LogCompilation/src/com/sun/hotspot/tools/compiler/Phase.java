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

public class Phase extends BasicLogEvent {

    private final int startNodes;
    private int endNodes;

    Phase(String n, double s, int nodes) {
        super(s, n);
        startNodes = nodes;
    }

    int getNodes() {
        return getStartNodes();
    }

    void setEndNodes(int n) {
        endNodes = n;
    }

    public String getName() {
        return getId();
    }

    public int getStartNodes() {
        return startNodes;
    }

    public int getEndNodes() {
        return endNodes;
    }

    @Override
    public void print(PrintStream stream) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
