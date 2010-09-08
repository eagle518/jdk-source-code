/*
 * @(#)XVerticalScrollbar.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import sun.awt.SunToolkit;
import java.util.logging.*;

/**
* A simple vertical scroll bar.
*/
class XVerticalScrollbar extends XScrollbar {
    public XVerticalScrollbar(XScrollbarClient sb) {
        super(ALIGNMENT_VERTICAL, sb);
    }

    public void setSize(int width, int height) {
        super.setSize(width, height);
        this.barWidth = width;
        this.barLength = height;
        calculateArrowWidth();
        rebuildArrows();
    }

    protected void rebuildArrows() {
        firstArrow = createArrowShape(true, true);
        secondArrow = createArrowShape(true, false);
    }

    boolean beforeThumb(int x, int y) {
        Rectangle pos = calculateThumbRect();
        return (y < pos.y);
    }

    protected Rectangle getThumbArea() {
        return new Rectangle(2, getArrowAreaWidth(), width-4, height - 2*getArrowAreaWidth());
    }
}
