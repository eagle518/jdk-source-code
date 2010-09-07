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
package com.sun.hotspot.igv.coordinator;

import com.sun.hotspot.igv.coordinator.actions.DiffGraphAction;
import com.sun.hotspot.igv.coordinator.actions.DiffGraphCookie;
import com.sun.hotspot.igv.coordinator.actions.RemoveCookie;
import com.sun.hotspot.igv.data.InputGraph;
import com.sun.hotspot.igv.data.services.GraphViewer;
import com.sun.hotspot.igv.data.services.InputGraphProvider;
import com.sun.hotspot.igv.util.PropertiesSheet;
import java.awt.Image;
import javax.swing.Action;
import org.openide.actions.OpenAction;
import org.openide.cookies.OpenCookie;
import org.openide.nodes.AbstractNode;
import org.openide.nodes.Children;
import org.openide.nodes.Node;
import org.openide.nodes.Sheet;
import org.openide.util.Lookup;
import org.openide.util.Utilities;
import org.openide.util.lookup.AbstractLookup;
import org.openide.util.lookup.InstanceContent;

/**
 *
 * @author Thomas Wuerthinger
 */
public class GraphNode extends AbstractNode {

    private InputGraph graph;

    /** Creates a new instance of GraphNode */
    public GraphNode(InputGraph graph) {
        this(graph, new InstanceContent());
    }

    private GraphNode(final InputGraph graph, InstanceContent content) {
        super(Children.LEAF, new AbstractLookup(content));
        this.graph = graph;
        this.setDisplayName(graph.getName());
        content.add(graph);

        final GraphViewer viewer = Lookup.getDefault().lookup(GraphViewer.class);

        if (viewer != null) {
            // Action for opening the graph
            content.add(new OpenCookie() {

                public void open() {
                    viewer.view(graph);
                }
            });
        }

        // Action for removing a graph
        content.add(new RemoveCookie() {

            public void remove() {
                graph.getGroup().removeGraph(graph);
            }
        });
    }

    @Override
    protected Sheet createSheet() {
        Sheet s = super.createSheet();
        PropertiesSheet.initializeSheet(graph.getProperties(), s);
        return s;
    }

    @Override
    public Image getIcon(int i) {
        return Utilities.loadImage("com/sun/hotspot/igv/coordinator/images/graph.gif");
    }

    @Override
    public Image getOpenedIcon(int i) {
        return getIcon(i);
    }

    @Override
    public <T extends Node.Cookie> T getCookie(Class<T> aClass) {
        if (aClass == DiffGraphCookie.class) {
            InputGraphProvider graphProvider = Utilities.actionsGlobalContext().lookup(InputGraphProvider.class);

            InputGraph graphA = null;
            if (graphProvider != null) {
                graphA = graphProvider.getGraph();
            }

            if (graphA != null && !graphA.isDifferenceGraph()) {
                return (T) new DiffGraphCookie(graphA, graph);
            }
        }

        return super.getCookie(aClass);
    }

    @Override
    public Action[] getActions(boolean b) {
        return new Action[]{(Action) DiffGraphAction.findObject(DiffGraphAction.class, true), (Action) OpenAction.findObject(OpenAction.class, true)};
    }

    @Override
    public Action getPreferredAction() {
        return (Action) OpenAction.findObject(OpenAction.class, true);
    }
}
