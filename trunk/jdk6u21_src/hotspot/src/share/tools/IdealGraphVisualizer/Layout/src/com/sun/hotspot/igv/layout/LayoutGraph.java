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

import java.util.HashSet;
import java.util.HashMap;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

/**
 *
 * @author Thomas Wuerthinger
 */
public class LayoutGraph {

    private Set<? extends Link> links;
    private SortedSet<Vertex> vertices;
    private HashMap<Vertex, Set<Port>> inputPorts;
    private HashMap<Vertex, Set<Port>> outputPorts;
    private HashMap<Port, Set<Link>> portLinks;

    public LayoutGraph(Set<? extends Link> links) {
        this(links, new HashSet<Vertex>());
    }

    public LayoutGraph(Set<? extends Link> links, Set<? extends Vertex> additionalVertices) {
        this.links = links;
        assert verify();

        vertices = new TreeSet<Vertex>();
        portLinks = new HashMap<Port, Set<Link>>();
        inputPorts = new HashMap<Vertex, Set<Port>>();
        outputPorts = new HashMap<Vertex, Set<Port>>();

        for (Link l : links) {
            Port p = l.getFrom();
            Port p2 = l.getTo();
            Vertex v1 = p.getVertex();
            Vertex v2 = p2.getVertex();

            if (!vertices.contains(v1)) {

                outputPorts.put(v1, new HashSet<Port>(1));
                inputPorts.put(v1, new HashSet<Port>(3));
                vertices.add(v1);
                assert vertices.contains(v1);
            }

            if (!vertices.contains(v2)) {
                vertices.add(v2);
                assert vertices.contains(v2);
                outputPorts.put(v2, new HashSet<Port>(1));
                inputPorts.put(v2, new HashSet<Port>(3));
            }

            if (!portLinks.containsKey(p)) {
                HashSet<Link> hashSet = new HashSet<Link>(3);
                portLinks.put(p, hashSet);
            }

            if (!portLinks.containsKey(p2)) {
                portLinks.put(p2, new HashSet<Link>(3));
            }

            outputPorts.get(v1).add(p);
            inputPorts.get(v2).add(p2);

            portLinks.get(p).add(l);
            portLinks.get(p2).add(l);
        }

        for (Vertex v : additionalVertices) {
            if (!vertices.contains(v)) {
                outputPorts.put(v, new HashSet<Port>(1));
                inputPorts.put(v, new HashSet<Port>(3));
                vertices.add(v);
                vertices.contains(v);
            }
        }
    }

    public Set<Port> getInputPorts(Vertex v) {
        return this.inputPorts.get(v);
    }

    public Set<Port> getOutputPorts(Vertex v) {
        return this.outputPorts.get(v);
    }

    public Set<Link> getPortLinks(Port p) {
        return portLinks.get(p);
    }

    public Set<? extends Link> getLinks() {
        return links;
    }

    public boolean verify() {
        return true;
    }

    public SortedSet<Vertex> getVertices() {
        return vertices;
    }

    private void markNotRoot(Set<Vertex> notRootSet, Vertex v, Vertex startingVertex) {

        if (notRootSet.contains(v)) {
            return;
        }
        if (v != startingVertex) {
            notRootSet.add(v);
        }
        Set<Port> outPorts = getOutputPorts(v);
        for (Port p : outPorts) {
            Set<Link> portLinks = getPortLinks(p);
            for (Link l : portLinks) {
                Port other = l.getTo();
                Vertex otherVertex = other.getVertex();
                if (otherVertex != startingVertex) {
                    markNotRoot(notRootSet, otherVertex, startingVertex);
                }
            }
        }
    }

    // Returns a set of vertices with the following properties:
    // - All Vertices in the set startingRoots are elements of the set.
    // - When starting a DFS at every vertex in the set, every vertex of the
    //   whole graph is visited.
    public Set<Vertex> findRootVertices(Set<Vertex> startingRoots) {

        Set<Vertex> notRootSet = new HashSet<Vertex>();
        for (Vertex v : startingRoots) {
            if (!notRootSet.contains(v)) {
                markNotRoot(notRootSet, v, v);
            }
        }

        Set<Vertex> tmpVertices = getVertices();
        for (Vertex v : tmpVertices) {
            if (!notRootSet.contains(v)) {
                if (this.getInputPorts(v).size() == 0) {
                    markNotRoot(notRootSet, v, v);
                }
            }
        }

        for (Vertex v : tmpVertices) {
            if (!notRootSet.contains(v)) {
                markNotRoot(notRootSet, v, v);
            }
        }

        Set<Vertex> result = new HashSet<Vertex>();
        for (Vertex v : tmpVertices) {
            if (!notRootSet.contains(v)) {
                result.add(v);
            }
        }
        assert tmpVertices.size() == 0 || result.size() > 0;
        return result;
    }

    public Set<Vertex> findRootVertices() {
        return findRootVertices(new HashSet<Vertex>());
    }

    public SortedSet<Cluster> getClusters() {

        SortedSet<Cluster> clusters = new TreeSet<Cluster>();
        for (Vertex v : getVertices()) {
            if (v.getCluster() != null) {
                clusters.add(v.getCluster());
            }
        }

        return clusters;
    }
}
