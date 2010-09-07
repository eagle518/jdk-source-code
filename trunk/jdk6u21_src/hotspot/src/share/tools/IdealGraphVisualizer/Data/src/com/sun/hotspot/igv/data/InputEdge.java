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
public class InputEdge {

    public enum State {

        SAME,
        NEW,
        DELETED
    }
    private char toIndex;
    private int from;
    private int to;
    private State state;

    public InputEdge(char toIndex, int from, int to) {
        this.toIndex = toIndex;
        this.from = from;
        this.to = to;
        this.state = State.SAME;
    }

    public State getState() {
        return state;
    }

    public void setState(State x) {
        this.state = x;
    }

    public char getToIndex() {
        return toIndex;
    }

    public String getName() {
        return "in" + toIndex;
    }

    public int getFrom() {
        return from;
    }

    public int getTo() {
        return to;
    }

    @Override
    public boolean equals(Object o) {
        if (o == null || !(o instanceof InputEdge)) {
            return false;
        }
        InputEdge conn2 = (InputEdge) o;
        return conn2.toIndex == toIndex && conn2.from == from && conn2.to == to;
    }

    @Override
    public String toString() {
        return "Edge from " + from + " to " + to + "(" + (int) toIndex + ") ";
    }

    @Override
    public int hashCode() {
        return (from << 20 | to << 8 | toIndex);
    }
}
