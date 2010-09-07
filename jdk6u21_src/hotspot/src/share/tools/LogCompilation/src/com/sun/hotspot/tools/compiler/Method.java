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

import java.util.Arrays;

public class Method implements Constants {

    private String holder;
    private String name;
    private String returnType;
    private String arguments;
    private String bytes;
    private String iicount;
    private String flags;

    String decodeFlags(int osr_bci) {
        int f = Integer.parseInt(getFlags());
        char[] c = new char[4];
        Arrays.fill(c, ' ');
        if (osr_bci >= 0) {
            c[0] = '%';
        }
        if ((f & JVM_ACC_SYNCHRONIZED) != 0) {
            c[1] = 's';
        }
        return new String(c);
    }

    String format(int osr_bci) {
        if (osr_bci >= 0) {
            return getHolder().replace('/', '.') + "::" + getName() + " @ " + osr_bci + " (" + getBytes() + " bytes)";
        } else {
            return getHolder().replace('/', '.') + "::" + getName() + " (" + getBytes() + " bytes)";
        }
    }

    @Override
    public String toString() {
        return getHolder().replace('/', '.') + "::" + getName() + " (" + getBytes() + " bytes)";
    }

    public String getHolder() {
        return holder;
    }

    public void setHolder(String holder) {
        this.holder = holder;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getReturnType() {
        return returnType;
    }

    public void setReturnType(String returnType) {
        this.returnType = returnType;
    }

    public String getArguments() {
        return arguments;
    }

    public void setArguments(String arguments) {
        this.arguments = arguments;
    }

    public String getBytes() {
        return bytes;
    }

    public void setBytes(String bytes) {
        this.bytes = bytes;
    }

    public String getIICount() {
        return iicount;
    }

    public void setIICount(String iicount) {
        this.iicount = iicount;
    }

    public String getFlags() {
        return flags;
    }

    public void setFlags(String flags) {
        this.flags = flags;
    }
}
