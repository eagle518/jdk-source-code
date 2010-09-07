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
import com.sun.hotspot.igv.graph.Connection.ConnectionStyle;
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
public class ColorFilter extends AbstractFilter {

    private List<ColorRule> colorRules;
    private String name;

    public ColorFilter(String name) {
        this.name = name;
        colorRules = new ArrayList<ColorRule>();
    }

    public String getName() {
        return name;
    }

    public void apply(Diagram diagram) {

        Properties.PropertySelector<Figure> selector = new Properties.PropertySelector<Figure>(diagram.getFigures());
        for (ColorRule rule : colorRules) {
            if (rule.getSelector() != null) {
                List<Figure> figures = rule.getSelector().selected(diagram);
                for (Figure f : figures) {
                    applyRule(rule, f);
                    if (rule.getColor() != null) {
                        f.setColor(rule.getColor());
                    }
                }
            } else {
                for (Figure f : diagram.getFigures()) {
                    applyRule(rule, f);
                }
            }
        }
    }

    private void applyRule(ColorRule rule, Figure f) {
        if (rule.getColor() != null) {
            f.setColor(rule.getColor());
        }
        Color color = rule.getLineColor();
        ConnectionStyle style = rule.getLineStyle();

        for (OutputSlot s : f.getOutputSlots()) {
            for (Connection c : s.getConnections()) {
                if (color != null) {
                    c.setColor(color);
                }

                if (style != null) {
                    c.setStyle(style);
                }
            }
        }
    }

    public void addRule(ColorRule r) {
        colorRules.add(r);
    }

    public static class ColorRule {

        private Color color;
        private Color lineColor;
        private Connection.ConnectionStyle lineStyle;
        private Selector selector;

        public ColorRule(Selector selector, Color c) {
            this(selector, c, null, null);
        }

        public ColorRule(Selector selector, Color c, Color lineColor, Connection.ConnectionStyle lineStyle) {
            this.selector = selector;
            this.color = c;
            this.lineColor = lineColor;
            this.lineStyle = lineStyle;

        }

        public ColorRule(Color c) {
            this(null, c);
        }

        public Color getColor() {
            return color;
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
