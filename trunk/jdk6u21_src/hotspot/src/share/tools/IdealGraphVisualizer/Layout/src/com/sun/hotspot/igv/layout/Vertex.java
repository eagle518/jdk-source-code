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
package com.sun.hotspot.igv.layout;

import java.awt.Dimension;
import java.awt.Point;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface Vertex extends Comparable<Vertex> {

    public Cluster getCluster();

    public Dimension getSize();

    public Point getPosition();

    public void setPosition(Point p);

    public boolean isRoot();
}
