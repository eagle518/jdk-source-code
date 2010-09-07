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
package com.sun.hotspot.igv.controlflow;

import com.sun.hotspot.igv.data.InputBlock;
import com.sun.hotspot.igv.layout.Cluster;
import com.sun.hotspot.igv.layout.Port;
import com.sun.hotspot.igv.layout.Vertex;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Point;
import org.netbeans.api.visual.border.BorderFactory;
import org.netbeans.api.visual.model.ObjectState;
import org.netbeans.api.visual.widget.LabelWidget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class BlockWidget extends LabelWidget implements Vertex {

    public static final Dimension SIZE = new Dimension(20, 20);
    private InputBlock block;
    private Port inputSlot;
    private Port outputSlot;
    private Cluster cluster;
    private boolean root;
    private static final Font font = new Font(Font.SERIF, Font.PLAIN, 12);
    private static final Font boldFont = font.deriveFont(Font.BOLD);
    public static final Color NORMAL_FOREGROUND_COLOR = Color.BLACK;
    public static final Color HOVER_FOREGROUND_COLOR = Color.BLUE;

    /** Creates a new instance of BlockWidget */
    public BlockWidget(ControlFlowScene scene, InputBlock block) {
        super(scene);
        this.block = block;
        this.setLabel(block.getName());
        this.setForeground(NORMAL_FOREGROUND_COLOR);
        this.setBorder(BorderFactory.createLineBorder(1, NORMAL_FOREGROUND_COLOR));
        this.setMinimumSize(SIZE);
        this.setMaximumSize(SIZE);

        this.setFont(font);

        final BlockWidget widget = this;
        inputSlot = new Port() {

            public Point getRelativePosition() {
                return new Point((int) (SIZE.getWidth() / 2), (int) (SIZE.getHeight() / 2));
            }

            public Vertex getVertex() {
                return widget;
            }
        };

        outputSlot = new Port() {

            public Point getRelativePosition() {
                return new Point((int) (SIZE.getWidth() / 2), (int) (SIZE.getHeight() / 2));
            }

            public Vertex getVertex() {
                return widget;
            }
        };
    }

    public Port getInputSlot() {
        return inputSlot;
    }

    public Port getOutputSlot() {
        return outputSlot;
    }

    public InputBlock getBlock() {
        return block;
    }

    public Dimension getSize() {
        return SIZE;
    }

    public void setPosition(Point p) {
        this.setPreferredLocation(p);
    }

    @Override
    public String toString() {
        return block.getName();
    }

    public Point getPosition() {
        return this.getPreferredLocation();
    }

    public Cluster getCluster() {
        return cluster;
    }

    public boolean isRoot() {
        return root;
    }

    public void setCluster(Cluster c) {
        cluster = c;
    }

    public void setRoot(boolean b) {
        root = b;
    }

    public int compareTo(Vertex o) {
        return toString().compareTo(o.toString());
    }

    @Override
    protected void notifyStateChanged(ObjectState previousState, ObjectState state) {
        super.notifyStateChanged(previousState, state);

        if (previousState.isHovered() != state.isHovered()) {
            if (state.isHovered()) {
                this.setBorder(BorderFactory.createLineBorder(1, HOVER_FOREGROUND_COLOR));
            } else {
                this.setBorder(BorderFactory.createLineBorder(1, NORMAL_FOREGROUND_COLOR));
            }
        }

        if (previousState.isSelected() != state.isSelected()) {
            if (state.isSelected()) {
                this.setFont(boldFont);
            } else {
                this.setFont(font);
            }
        }
    }
}
