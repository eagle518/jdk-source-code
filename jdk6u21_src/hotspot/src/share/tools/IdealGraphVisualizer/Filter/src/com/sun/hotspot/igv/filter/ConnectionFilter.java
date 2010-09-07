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
package com.sun.hotspot.igv.filter;

import com.sun.hotspot.igv.graph.Connection;
import com.sun.hotspot.igv.graph.Diagram;
import com.sun.hotspot.igv.graph.Figure;
import com.sun.hotspot.igv.graph.OutputSlot;
import com.sun.hotspot.igv.graph.Selector;
import com.sun.hotspot.igv.data.Properties;
import java.awt.Color;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ConnectionFilter extends AbstractFilter {

    private List<ConnectionStyleRule> connectionStyleRules;
    private String name;

    public ConnectionFilter(String name) {
        this.name = name;
        connectionStyleRules = new ArrayList<ConnectionStyleRule>();
    }

    public String getName() {
        return name;
    }

    public void apply(Diagram diagram) {

        Properties.PropertySelector<Figure> selector = new Properties.PropertySelector<Figure>(diagram.getFigures());
        for (ConnectionStyleRule rule : connectionStyleRules) {
            List<Figure> figures = null;
            if (rule.getSelector() != null) {
                figures = rule.getSelector().selected(diagram);
            } else {
                figures = diagram.getFigures();
            }

            for (Figure f : figures) {
                for (OutputSlot os : f.getOutputSlots()) {
                    for (Connection c : os.getConnections()) {
                        if (figures.contains(c.getInputSlot().getFigure())) {
                            c.setStyle(rule.getLineStyle());
                            c.setColor(rule.getLineColor());
                        }
                    }
                }
            }
        }
    }

    public void addRule(ConnectionStyleRule r) {
        connectionStyleRules.add(r);
    }

    public static class ConnectionStyleRule {

        private Color lineColor;
        private Connection.ConnectionStyle lineStyle;
        private Selector selector;

        public ConnectionStyleRule(Selector selector, Color lineColor, Connection.ConnectionStyle lineStyle) {
            this.selector = selector;
            this.lineColor = lineColor;
            this.lineStyle = lineStyle;
        }

        public Selector getSelector() {
            return selector;
        }

        public Color getLineColor() {
            return lineColor;
        }

        public Connection.ConnectionStyle getLineStyle() {
            return lineStyle;
        }
    }
}
