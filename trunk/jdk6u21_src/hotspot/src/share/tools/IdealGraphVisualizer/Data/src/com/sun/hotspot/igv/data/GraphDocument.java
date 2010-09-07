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
package com.sun.hotspot.igv.data;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class GraphDocument extends Properties.Entity implements ChangedEventProvider<GraphDocument> {

    private List<Group> groups;
    private ChangedEvent<GraphDocument> changedEvent;

    public GraphDocument() {
        groups = new ArrayList<Group>();
        changedEvent = new ChangedEvent<GraphDocument>(this);
    }

    public void clear() {
        groups.clear();
        getChangedEvent().fire();
    }

    public ChangedEvent<GraphDocument> getChangedEvent() {
        return changedEvent;
    }

    public List<Group> getGroups() {
        return Collections.unmodifiableList(groups);
    }

    public void addGroup(Group group) {
        group.setDocument(this);
        groups.add(group);
        getChangedEvent().fire();
    }

    public void removeGroup(Group group) {
        if (groups.contains(group)) {
            group.setDocument(null);
            groups.remove(group);
            getChangedEvent().fire();
        }
    }

    public void addGraphDocument(GraphDocument document) {
        for (Group g : document.groups) {
            this.addGroup(g);
        }
        document.clear();
        getChangedEvent().fire();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("GraphDocument: " + getProperties().toString() + " \n\n");
        for (Group g : getGroups()) {
            sb.append(g.toString());
            sb.append("\n\n");
        }

        return sb.toString();
    }
}
