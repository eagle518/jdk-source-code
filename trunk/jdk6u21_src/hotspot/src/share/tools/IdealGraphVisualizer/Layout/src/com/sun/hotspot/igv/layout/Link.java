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

import java.awt.Point;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface Link {

    public Port getFrom();

    public Port getTo();

    public List<Point> getControlPoints();

    public void setControlPoints(List<Point> list);
}
