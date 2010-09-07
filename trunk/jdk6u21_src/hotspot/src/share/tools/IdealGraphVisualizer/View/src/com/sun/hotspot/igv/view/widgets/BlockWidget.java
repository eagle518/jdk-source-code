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
package com.sun.hotspot.igv.view.widgets;

import com.sun.hotspot.igv.data.InputBlock;
import com.sun.hotspot.igv.graph.Diagram;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Stroke;
import java.awt.geom.Rectangle2D;
import org.netbeans.api.visual.widget.Scene;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class BlockWidget extends Widget {

    public static final int BORDER = 20;
    public static final Color BACKGROUND_COLOR = new Color(235, 235, 255);
    private static final Font titleFont = new Font("Serif", Font.PLAIN, 14).deriveFont(Font.BOLD);
    private InputBlock blockNode;
    private Diagram diagram;

    public BlockWidget(Scene scene, Diagram d, InputBlock blockNode) {
        super(scene);
        this.blockNode = blockNode;
        this.diagram = d;
        this.setBackground(BACKGROUND_COLOR);
        this.setOpaque(true);
        this.setCheckClipping(true);
    }

    @Override
    protected void paintWidget() {
        super.paintWidget();
        Graphics2D g = this.getGraphics();
        Stroke old = g.getStroke();
        g.setColor(Color.BLUE);
        Rectangle r = new Rectangle(this.getPreferredBounds());
        r.width--;
        r.height--;
        if (this.getBounds().width > 0 && this.getBounds().height > 0) {
            g.setStroke(new BasicStroke(2));
            g.drawRect(r.x, r.y, r.width, r.height);
        }

        Color titleColor = Color.BLACK;
        g.setColor(titleColor);
        g.setFont(titleFont);

        String s = "B" + blockNode.toString();
        Rectangle2D r1 = g.getFontMetrics().getStringBounds(s, g);
        g.drawString(s, r.x + 5, r.y + (int) r1.getHeight());
        g.setStroke(old);
    }
}
