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

import com.sun.hotspot.igv.view.widgets.SlotWidget;
import java.awt.Point;
import java.awt.Rectangle;
import org.netbeans.api.visual.anchor.Anchor;
import org.netbeans.api.visual.anchor.Anchor.Entry;
import org.netbeans.api.visual.anchor.Anchor.Result;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ConnectionAnchor extends Anchor {

    public enum HorizontalAlignment {

        Left,
        Center,
        Right
    }
    private HorizontalAlignment alignment;

    public ConnectionAnchor(Widget widget) {
        this(HorizontalAlignment.Center, widget);
    }

    public ConnectionAnchor(HorizontalAlignment alignment, Widget widget) {
        super(widget);
        this.alignment = alignment;
    }

    public Result compute(Entry entry) {
        return new Result(getRelatedSceneLocation(), Anchor.DIRECTION_ANY);
    }

    @Override
    public Point getRelatedSceneLocation() {
        Point p = null;
        Widget w = getRelatedWidget();
        if (w != null) {
            if (w instanceof SlotWidget) {
                p = ((SlotWidget) w).getAnchorPosition();
            } else {
                Rectangle r = w.convertLocalToScene(w.getBounds());
                int y = r.y + r.height / 2;
                int x = r.x;
                if (alignment == HorizontalAlignment.Center) {
                    x = r.x + r.width / 2;
                } else if (alignment == HorizontalAlignment.Right) {
                    x = r.x + r.width;
                }

                p = new Point(x, y);
            }
        }

        return p;
    }
}
