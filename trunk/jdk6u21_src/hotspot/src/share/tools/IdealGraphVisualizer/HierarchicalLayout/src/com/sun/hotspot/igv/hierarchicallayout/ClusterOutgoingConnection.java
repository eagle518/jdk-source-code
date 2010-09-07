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
package com.sun.hotspot.igv.hierarchicallayout;

import com.sun.hotspot.igv.layout.Link;
import com.sun.hotspot.igv.layout.Port;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ClusterOutgoingConnection implements Link {

    private List<Point> intermediatePoints;
    private ClusterOutputSlotNode outputSlotNode;
    private Link connection;
    private Port inputSlot;
    private Port outputSlot;

    public ClusterOutgoingConnection(ClusterOutputSlotNode outputSlotNode, Link c) {
        this.outputSlotNode = outputSlotNode;
        this.connection = c;
        this.intermediatePoints = new ArrayList<Point>();

        outputSlot = c.getFrom();
        inputSlot = outputSlotNode.getInputSlot();
    }

    public Port getTo() {
        return inputSlot;
    }

    public Port getFrom() {
        return outputSlot;
    }

    public void setControlPoints(List<Point> p) {
        this.intermediatePoints = p;
    }

    public List<Point> getControlPoints() {
        return intermediatePoints;
    }
}
