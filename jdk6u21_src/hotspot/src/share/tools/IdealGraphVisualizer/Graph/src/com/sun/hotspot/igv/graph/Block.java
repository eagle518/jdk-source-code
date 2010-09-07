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

import com.sun.hotspot.igv.data.InputBlock;
import com.sun.hotspot.igv.layout.Cluster;
import java.awt.Rectangle;
import java.util.HashSet;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Block implements Cluster {

    private InputBlock inputBlock;
    private Rectangle bounds;
    private Diagram diagram;

    public Block(InputBlock inputBlock, Diagram diagram) {
        this.inputBlock = inputBlock;
        this.diagram = diagram;
    }

    public Cluster getOuter() {
        return null;
    }

    public InputBlock getInputBlock() {
        return inputBlock;
    }

    public Set<? extends Cluster> getSuccessors() {
        Set<Block> succs = new HashSet<Block>();
        for (InputBlock b : inputBlock.getSuccessors()) {
            succs.add(diagram.getBlock(b));
        }
        return succs;
    }

    public Set<? extends Cluster> getPredecessors() {
        Set<Block> succs = new HashSet<Block>();
        for (InputBlock b : inputBlock.getPredecessors()) {
            succs.add(diagram.getBlock(b));
        }
        return succs;
    }

    public void setBounds(Rectangle r) {
        this.bounds = r;
    }

    public Rectangle getBounds() {
        return bounds;
    }

    public int compareTo(Cluster o) {
        return toString().compareTo(o.toString());
    }
}
