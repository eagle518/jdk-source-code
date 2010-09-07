/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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
package com.sun.hotspot.igv.data;

/**
 *
 * @author Thomas Wuerthinger
 */
public class InputBytecode {

    private int bci;
    private String name;
    private InputMethod inlined;

    public InputBytecode(int bci, String name) {
        this.bci = bci;
        this.name = name;
    }

    public InputMethod getInlined() {
        return inlined;
    }

    public void setInlined(InputMethod inlined) {
        this.inlined = inlined;
    }

    public int getBci() {
        return bci;
    }

    public String getName() {
        return name;
    }
}
