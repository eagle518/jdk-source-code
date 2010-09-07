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

public class NMethod extends BasicLogEvent {

    private long address;
    private long size;

    NMethod(double s, String i, long a, long sz) {
        super(s, i);
        address = a;
        size = sz;
    }

    public void print(PrintStream out) {
        // XXX Currently we do nothing
        // throw new InternalError();
    }

    public long getAddress() {
        return address;
    }

    public void setAddress(long address) {
        this.address = address;
    }

    public long getSize() {
        return size;
    }

    public void setSize(long size) {
        this.size = size;
    }
}
