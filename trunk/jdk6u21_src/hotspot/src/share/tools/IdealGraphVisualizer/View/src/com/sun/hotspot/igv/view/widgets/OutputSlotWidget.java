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

import com.sun.hotspot.igv.graph.OutputSlot;
import com.sun.hotspot.igv.view.DiagramScene;
import java.awt.Point;
import java.util.List;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class OutputSlotWidget extends SlotWidget {

    private OutputSlot outputSlot;

    public OutputSlotWidget(OutputSlot slot, DiagramScene scene, Widget parent, FigureWidget fw) {
        super(slot, scene, parent, fw);
        outputSlot = slot;
        init();
        getFigureWidget().getRightWidget().addChild(this);
    }

    public OutputSlot getOutputSlot() {
        return outputSlot;
    }

    protected Point calculateRelativeLocation() {
        if (getFigureWidget().getBounds() == null) {
            return new Point(0, 0);
        }

        double x = this.getFigureWidget().getBounds().width;
        List<OutputSlot> slots = outputSlot.getFigure().getOutputSlots();
        assert slots.contains(outputSlot);
        return new Point((int) x, (int) (calculateRelativeY(slots.size(), slots.indexOf(outputSlot))));
    }
}
