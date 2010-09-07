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
package com.sun.hotspot.igv.view.widgets;

import com.sun.hotspot.igv.graph.InputSlot;
import com.sun.hotspot.igv.view.DiagramScene;
import java.awt.Point;
import java.util.List;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class InputSlotWidget extends SlotWidget {

    private InputSlot inputSlot;

    public InputSlotWidget(InputSlot slot, DiagramScene scene, Widget parent, FigureWidget fw) {
        super(slot, scene, parent, fw);
        inputSlot = slot;
        init();
        getFigureWidget().getLeftWidget().addChild(this);
    }

    public InputSlot getInputSlot() {
        return inputSlot;
    }

    protected Point calculateRelativeLocation() {
        if (getFigureWidget().getBounds() == null) {
            return new Point(0, 0);
        }

        double x = 0;
        List<InputSlot> slots = inputSlot.getFigure().getInputSlots();
        assert slots.contains(inputSlot);
        return new Point((int) x, (int) (calculateRelativeY(slots.size(), slots.indexOf(inputSlot))));
    }
}
