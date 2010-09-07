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
import com.sun.hotspot.igv.data.InputGraph;
import com.sun.hotspot.igv.data.InputNode;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Source {

    private List<InputNode> sourceNodes;
    private Set<Integer> set;

    public Source() {
        sourceNodes = new ArrayList<InputNode>(1);
    }

    public List<InputNode> getSourceNodes() {
        return Collections.unmodifiableList(sourceNodes);
    }

    public Set<Integer> getSourceNodesAsSet() {
        if (set == null) {
            set = new HashSet<Integer>();
            for (InputNode n : sourceNodes) {
                int id = n.getId();
                //if(id < 0) id = -id;
                set.add(id);
            }
        }
        return set;
    }

    public void addSourceNode(InputNode n) {
        sourceNodes.add(n);
        set = null;
    }

    public void removeSourceNode(InputNode n) {
        sourceNodes.remove(n);
        set = null;
    }

    public interface Provider {

        public Source getSource();
    }

    public void setSourceNodes(List<InputNode> sourceNodes) {
        this.sourceNodes = sourceNodes;
        set = null;
    }

    public void addSourceNodes(Source s) {
        for (InputNode n : s.getSourceNodes()) {
            sourceNodes.add(n);
        }
        set = null;
    }

    public boolean isInBlock(InputGraph g, InputBlock blockNode) {

        for (InputNode n : this.getSourceNodes()) {
            if (g.getBlock(n) == blockNode) {
                return true;
            }
        }
        return false;
    }
}
