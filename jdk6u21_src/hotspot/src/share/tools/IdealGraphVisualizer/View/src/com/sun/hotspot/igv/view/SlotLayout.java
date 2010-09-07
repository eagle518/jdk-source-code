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
package com.sun.hotspot.igv.view;

import java.awt.Point;
import java.awt.Rectangle;
import java.util.Collection;
import java.util.List;
import org.netbeans.api.visual.layout.Layout;
import org.netbeans.api.visual.layout.LayoutFactory;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class SlotLayout implements Layout {

    public enum HorizontalAlignment {

        Left,
        Center,
        Right
    }
    private Layout baseLayout;
    private HorizontalAlignment alignment;
    private boolean vertical;

    public SlotLayout() {
        this(HorizontalAlignment.Center, false);
    }

    public SlotLayout(HorizontalAlignment alignment, boolean vertical) {
        this.alignment = alignment;
        baseLayout = LayoutFactory.createVerticalFlowLayout();
        this.vertical = vertical;
    }

    public void layout(Widget widget) {
        if (!vertical) {
            Collection<Widget> children = widget.getChildren();
            int gap = 0;
            int max = 0;
            for (Widget child : children) {
                Rectangle preferredBounds = child.getPreferredBounds();
                int i = preferredBounds.width;
                if (i > max) {
                    max = i;
                }
            }
            int pos = 0;
            for (Widget child : children) {
                Rectangle preferredBounds = child.getPreferredBounds();
                int x = preferredBounds.x;
                int y = preferredBounds.y;
                int width = preferredBounds.width;
                int height = preferredBounds.height;
                if (pos == 0) {
                    pos += height / 2;
                }
                int lx = -x;
                int ly = pos - y;
                switch (alignment) {
                    case Center:
                        lx += (max - width) / 2;
                        break;
                    case Left:
                        break;
                    case Right:
                        lx += max - width;
                        break;
                }
                child.resolveBounds(new Point(lx, ly), new Rectangle(x, y, width, height));
                pos += height + gap;
            }
        } else {

            Collection<Widget> children = widget.getChildren();
            int gap = 0;
            int max = 0;
            for (Widget child : children) {
                Rectangle preferredBounds = child.getPreferredBounds();
                int i = preferredBounds.height;
                if (i > max) {
                    max = i;
                }
            }
            int pos = 0;
            for (Widget child : children) {
                Rectangle preferredBounds = child.getPreferredBounds();
                int x = preferredBounds.x;
                int y = preferredBounds.y;
                int width = preferredBounds.width;
                int height = preferredBounds.height;
                if (pos == 0) {
                    pos += width / 2;
                }
                int lx = pos - x;
                int ly = -y;
                switch (alignment) {
                    case Center:
                        ly += (max - height) / 2;
                        break;
                    case Left:
                        break;
                    case Right:
                        ly += max - height;
                        break;
                }
                child.resolveBounds(new Point(lx, ly), new Rectangle(x, y, width, height));
                pos += width + gap;
            }

        }
    }

    public boolean requiresJustification(Widget widget) {
        return true;
    }

    public void justify(Widget widget) {
        baseLayout.justify(widget);

        Rectangle client = widget.getClientArea();
        List<Widget> children = widget.getChildren();

        int count = children.size();
        int z = 0;

        int maxWidth = 0;
        for (Widget c : children) {
            if (c.getPreferredBounds().width > maxWidth) {
                maxWidth = c.getPreferredBounds().width;
            }
        }

        for (Widget c : children) {
            z++;
            Point curLocation = c.getLocation();
            Rectangle curBounds = c.getBounds();


            Point location = new Point(curLocation.x, client.y + client.height * z / (count + 1) - curBounds.height / 2);
            if (vertical) {
                location = new Point(client.x + client.width * z / (count + 1) - maxWidth / 2, curLocation.y);
            }
            c.resolveBounds(location, null);
        }
    }
}
