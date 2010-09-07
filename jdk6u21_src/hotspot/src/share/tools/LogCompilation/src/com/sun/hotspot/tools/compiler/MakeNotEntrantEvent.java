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

class MakeNotEntrantEvent extends BasicLogEvent {
    private final boolean zombie;

    private NMethod nmethod;

    MakeNotEntrantEvent(double s, String i, boolean z, NMethod nm) {
        super(s, i);
        zombie = z;
        nmethod = nm;
    }

    public NMethod getNMethod() {
        return nmethod;
    }

    public void print(PrintStream stream) {
        if (isZombie()) {
            stream.printf("%s make_zombie\n", getId());
        } else {
            stream.printf("%s make_not_entrant\n", getId());
        }
    }

    public boolean isZombie() {
        return zombie;
    }
}
