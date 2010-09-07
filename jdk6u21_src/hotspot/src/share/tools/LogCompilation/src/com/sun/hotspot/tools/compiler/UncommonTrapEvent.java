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

class UncommonTrapEvent extends BasicLogEvent {

    private final String reason;
    private final String action;
    private int count;
    private String jvms = "";

    UncommonTrapEvent(double s, String i, String r, String a, int c) {
        super(s, i);
        reason = r;
        action = a;
        count = c;
    }


    public void addJVMS(String method, int bci) {
        setJvms(getJvms() + "  @" + bci + " " + method + "\n");
    }

    public void updateCount(UncommonTrapEvent trap) {
        setCount(Math.max(getCount(), trap.getCount()));
    }

    public void print(PrintStream stream) {
        stream.printf("%s uncommon trap %s %s\n", getId(), getReason(), getAction());
        stream.print(getJvms());
    }

    public String getReason() {
        return reason;
    }

    public String getAction() {
        return action;
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }

    public String getJvms() {
        return jvms;
    }

    public void setJvms(String jvms) {
        this.jvms = jvms;
    }

    public void setCompilation(Compilation compilation) {
        this.compilation = compilation;
    }
}
