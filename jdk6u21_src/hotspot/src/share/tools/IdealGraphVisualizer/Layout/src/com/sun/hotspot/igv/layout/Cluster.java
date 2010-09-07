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

import java.awt.Rectangle;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface Cluster extends Comparable<Cluster> {

    public Cluster getOuter();

    public void setBounds(Rectangle r);

    public Set<? extends Cluster> getSuccessors();

    public Set<? extends Cluster> getPredecessors();
}
