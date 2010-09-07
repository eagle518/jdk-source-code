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

/**
 *
 * @author Thomas Wuerthinger
 */
public class Edge<N, E> {

    private E data;
    private Node<N, E> source;
    private Node<N, E> dest;

    protected Edge(Graph<N, E> graph, Node<N, E> source, Node<N, E> dest, E data) {
        setData(data);
        this.source = source;
        this.dest = dest;
        assert source != null;
        assert dest != null;
        assert source.getGraph() == dest.getGraph();
        assert source.getGraph() != null;
        assert dest.getGraph() != null;
    }

    public Node<N, E> getSource() {
        return source;
    }

    public Node<N, E> getDest() {
        return dest;
    }

    public E getData() {
        return data;
    }

    public void setData(E e) {
        data = e;
    }

    public void remove() {
        source.getGraph().removeEdge(this, null);
    }

    public boolean isSelfLoop() {
        return source == dest;
    }

    public void reverse() {

        // Remove from current source / dest
        source.removeOutEdge(this);
        dest.removeInEdge(this);

        Node<N, E> tmp = source;
        source = dest;
        dest = tmp;

        // Add to new source / dest
        source.addOutEdge(this);
        dest.addInEdge(this);
    }

    public String toString() {
        return "Edge (" + source + " -- " + dest + "): " + data;
    }
}
