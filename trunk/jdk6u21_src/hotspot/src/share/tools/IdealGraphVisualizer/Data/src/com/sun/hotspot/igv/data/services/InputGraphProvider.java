/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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
 *
 */
package com.sun.hotspot.igv.data.services;

import com.sun.hotspot.igv.data.InputGraph;
import com.sun.hotspot.igv.data.InputNode;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface InputGraphProvider {

    InputGraph getGraph();

    void setSelectedNodes(Set<InputNode> nodes);
}
