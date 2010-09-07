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

import com.sun.hotspot.igv.layout.Port;
import com.sun.hotspot.igv.layout.Vertex;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Comparator;

/**
 *
 * @author Thomas Wuerthinger
 */
public abstract class Slot implements Port, Source.Provider {

    private int wantedIndex;
    private String name;
    private String shortName; // 1 - 2 characters
    private Source source;
    protected List<Connection> connections;
    private Figure figure;

    protected Slot(Figure figure, int wantedIndex) {
        this.figure = figure;
        connections = new ArrayList<Connection>(2);
        source = new Source();
        this.wantedIndex = wantedIndex;
        name = "";
        shortName = "";
        assert figure != null;
    }
    public static final Comparator<Slot> slotIndexComparator = new Comparator<Slot>() {

        public int compare(Slot o1, Slot o2) {
            return o1.wantedIndex - o2.wantedIndex;
        }
    };
    public static final Comparator<Slot> slotFigureComparator = new Comparator<Slot>() {

        public int compare(Slot o1, Slot o2) {
            return o1.figure.getId() - o2.figure.getId();
        }
    };

    public int getWantedIndex() {
        return wantedIndex;
    }

    public Source getSource() {
        return source;
    }

    public String getName() {
        return name;
    }

    public void setShortName(String s) {
        assert s != null;
        assert s.length() <= 2;
        this.shortName = s;

    }

    public String getShortName() {
        return shortName;
    }

    public boolean getShowName() {
        return getShortName() != null && getShortName().length() > 0;
    }

    public void setName(String s) {
        if (s == null) {
            s = "";
        }
        this.name = s;
    }

    public Figure getFigure() {
        assert figure != null;
        return figure;
    }

    public List<Connection> getConnections() {
        return Collections.unmodifiableList(connections);
    }

    public void removeAllConnections() {
        List<Connection> connectionsCopy = new ArrayList<Connection>(this.connections);
        for (Connection c : connectionsCopy) {
            c.remove();
        }
    }

    public Vertex getVertex() {
        return figure;
    }

    public abstract int getPosition();

    public abstract void setPosition(int position);
}
