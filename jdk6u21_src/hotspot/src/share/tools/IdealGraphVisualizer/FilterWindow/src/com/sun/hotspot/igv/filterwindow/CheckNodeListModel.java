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
package com.sun.hotspot.igv.filterwindow;

import org.openide.explorer.view.NodeListModel;
import org.openide.nodes.Node;

/**
 *
 * @author Thomas Wuerthinger
 */
public class CheckNodeListModel extends NodeListModel {

    private Node rootNode;

    @Override
    public void setNode(Node rootNode) {
        this.rootNode = rootNode;
        super.setNode(rootNode);
    }

    public CheckNode getCheckNodeAt(int index) {
        return (CheckNode) rootNode.getChildren().getNodes()[index];
    }
}
