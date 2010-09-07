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

import com.sun.hotspot.igv.layout.Link;
import com.sun.hotspot.igv.layout.Port;
import java.awt.Color;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Connection implements Source.Provider, Link {

    public enum ConnectionStyle {

        NORMAL,
        DASHED,
        BOLD
    }
    private InputSlot inputSlot;
    private OutputSlot outputSlot;
    private Source source;
    private Color color;
    private ConnectionStyle style;
    private List<Point> controlPoints;

    protected Connection(InputSlot inputSlot, OutputSlot outputSlot) {
        this.inputSlot = inputSlot;
        this.outputSlot = outputSlot;
        this.inputSlot.connections.add(this);
        this.outputSlot.connections.add(this);
        controlPoints = new ArrayList<Point>();
        Figure sourceFigure = this.outputSlot.getFigure();
        Figure destFigure = this.inputSlot.getFigure();
        sourceFigure.addSuccessor(destFigure);
        destFigure.addPredecessor(sourceFigure);
        source = new Source();

        this.color = Color.BLACK;
        this.style = ConnectionStyle.NORMAL;
    }

    public InputSlot getInputSlot() {
        return inputSlot;
    }

    public OutputSlot getOutputSlot() {
        return outputSlot;
    }

    public Color getColor() {
        return color;
    }

    public ConnectionStyle getStyle() {
        return style;
    }

    public void setColor(Color c) {
        color = c;
    }

    public void setStyle(ConnectionStyle s) {
        style = s;
    }

    public Source getSource() {
        return source;
    }

    public void remove() {
        inputSlot.getFigure().removePredecessor(outputSlot.getFigure());
        inputSlot.connections.remove(this);
        outputSlot.getFigure().removeSuccessor(inputSlot.getFigure());
        outputSlot.connections.remove(this);
    }

    @Override
    public String toString() {
        return "Connection(" + getFrom().getVertex() + " to " + getTo().getVertex() + ")";
    }

    public Port getFrom() {
        return outputSlot;
    }

    public Port getTo() {
        return inputSlot;
    }

    public List<Point> getControlPoints() {
        return controlPoints;
    }

    public void setControlPoints(List<Point> list) {
        controlPoints = list;
    }
}
