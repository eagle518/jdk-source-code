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

import com.sun.hotspot.igv.layout.Cluster;
import com.sun.hotspot.igv.layout.Port;
import com.sun.hotspot.igv.layout.Vertex;
import java.awt.Dimension;
import java.awt.Point;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ClusterOutputSlotNode implements Vertex {

    private final int SIZE = 0;
    private Point position;
    private Port inputSlot;
    private Port outputSlot;
    private ClusterNode blockNode;
    private boolean root;
    private Cluster cluster;
    private ClusterOutgoingConnection conn;
    private String id;

    public void setOutgoingConnection(ClusterOutgoingConnection c) {
        this.conn = c;
    }

    public ClusterOutgoingConnection getOutgoingConnection() {
        return conn;
    }

    @Override
    public String toString() {
        return id;
    }

    public ClusterOutputSlotNode(ClusterNode n, String id) {
        this.blockNode = n;
        this.id = id;

        n.addSubNode(this);

        final Vertex thisNode = this;
        final ClusterNode thisBlockNode = blockNode;

        inputSlot = new Port() {

            public Point getRelativePosition() {
                return new Point(0, 0);
            }

            public Vertex getVertex() {
                return thisNode;
            }

            @Override
            public String toString() {
                return "InPort of " + thisNode.toString();
            }
        };

        outputSlot = new Port() {

            public Point getRelativePosition() {
                Point p = new Point(thisNode.getPosition());
                p.x += ClusterNode.BORDER;
                p.y = thisBlockNode.getSize().height;
                return p;
            }

            public Vertex getVertex() {
                return thisBlockNode;
            }

            @Override
            public String toString() {
                return "OutPort of " + thisNode.toString();
            }
        };
    }

    public Dimension getSize() {
        return new Dimension(SIZE, SIZE);
    }

    public void setPosition(Point p) {
        this.position = p;
    }

    public Point getPosition() {
        return position;
    }

    public Port getInputSlot() {
        return inputSlot;
    }

    public Port getOutputSlot() {
        return outputSlot;
    }

    public void setCluster(Cluster c) {
        cluster = c;
    }

    public void setRoot(boolean b) {
        root = b;
    }

    public Cluster getCluster() {
        return cluster;
    }

    public boolean isRoot() {
        return root;
    }

    public int compareTo(Vertex o) {
        return toString().compareTo(o.toString());
    }
}
