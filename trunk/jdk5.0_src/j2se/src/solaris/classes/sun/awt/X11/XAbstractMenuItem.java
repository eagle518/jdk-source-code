/*
 * @(#)XAbstractMenuItem.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Graphics;

public interface XAbstractMenuItem {
    int getWidth(Graphics g);
    int getShortcutWidth(Graphics g);
    String getLabel();
    int getHeight(Graphics g);
    void paint(Graphics g, int top, int bottom, int width, int shortcutOffset, boolean selected);
    void setMenuPeer(XMenuPeer parentMenu);
}
