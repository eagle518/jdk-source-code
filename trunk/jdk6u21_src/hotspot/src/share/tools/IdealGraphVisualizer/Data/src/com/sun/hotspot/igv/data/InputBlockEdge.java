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
public class InputBlockEdge {

    private InputBlock from;
    private InputBlock to;

    public InputBlockEdge(InputBlock from, InputBlock to) {
        assert from != null;
        assert to != null;
        this.from = from;
        this.to = to;
    }

    public InputBlock getFrom() {
        return from;
    }

    public InputBlock getTo() {
        return to;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof InputBlockEdge && obj != null) {
            InputBlockEdge e = (InputBlockEdge) obj;
            return e.from.equals(from) && e.to.equals(to);
        }
        return super.equals(obj);
    }

    @Override
    public int hashCode() {
        int hash = from.hashCode();
        hash = 59 * hash + to.hashCode();
        return hash;
    }
}
