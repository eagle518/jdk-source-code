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
package com.sun.hotspot.igv.view;

import com.sun.hotspot.igv.data.InputGraph;
import com.sun.hotspot.igv.data.services.InputGraphProvider;
import com.sun.hotspot.igv.data.InputNode;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public class EditorInputGraphProvider implements InputGraphProvider {

    public InputGraph getGraph() {
        EditorTopComponent e = EditorTopComponent.getActive();
        if (e == null) {
            return null;
        }
        return e.getDiagramModel().getGraphToView();
    }

    public void setSelectedNodes(Set<InputNode> nodes) {
        EditorTopComponent e = EditorTopComponent.getActive();
        if (e != null) {
            e.setSelectedNodes(nodes);
        }
    }
}
