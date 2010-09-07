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
package com.sun.hotspot.igv.view;

import com.sun.hotspot.igv.data.InputGraph;
import com.sun.hotspot.igv.data.services.GraphViewer;
import com.sun.hotspot.igv.graph.Diagram;
import com.sun.hotspot.igv.settings.Settings;

/**
 *
 * @author Thomas Wuerthinger
 */
public class GraphViewerImplementation implements GraphViewer {

    public void view(InputGraph graph) {
        Diagram diagram = Diagram.createDiagram(graph, Settings.get().get(Settings.NODE_TEXT, Settings.NODE_TEXT_DEFAULT));
        EditorTopComponent tc = new EditorTopComponent(diagram);
        tc.open();
        tc.requestActive();
    }
}
