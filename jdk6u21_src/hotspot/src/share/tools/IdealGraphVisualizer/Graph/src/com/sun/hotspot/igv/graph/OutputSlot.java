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
package com.sun.hotspot.igv.graph;

import java.awt.Point;

/**
 *
 * @author Thomas Wuerthinger
 */
public class OutputSlot extends Slot {

    protected OutputSlot(Figure figure, int wantedIndex) {
        super(figure, wantedIndex);
    }

    public int getPosition() {
        return getFigure().getOutputSlots().indexOf(this);
    }

    public void setPosition(int position) {
        OutputSlot s = getFigure().outputSlots.remove(position);
        getFigure().outputSlots.add(position, s);
    }

    public Point getRelativePosition() {
        return new Point(getFigure().getWidth() * (getPosition() + 1) / (getFigure().getOutputSlots().size() + 1), getFigure().getSize().height - Figure.SLOT_WIDTH + Figure.SLOT_START);
    }

    @Override
    public String toString() {
        return "OutputSlot[figure=" + this.getFigure().toString() + ", position=" + getPosition() + "]";
    }
}
