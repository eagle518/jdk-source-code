/*
 * @(#)XWarningWindow.java	1.12 04/03/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import sun.java2d.SunGraphics2D;

class XWarningWindow extends XWindow {
    final static int defaultHeight = 27;
    
    Window ownerWindow;
    XWarningWindow(Window ownerWindow, long parentWindow) {
        super(ownerWindow, parentWindow);
        this.ownerWindow = ownerWindow;        
        xSetVisible(true);
        toFront();
    }
    
    protected String getWMName() {
        return "Warning window";
    }

    public Graphics getGraphics() {
        if ((surfaceData == null) || (ownerWindow == null)) return null;
        return getGraphics(surfaceData,
                                 getColor(),
                                 getBackground(),
                                 getFont());
    }
    void paint(Graphics g, int x, int y, int width, int height) {
        String warningString = getWarningString();
        Rectangle bounds = getBounds();
        bounds.x = 0;
        bounds.y = 0;
        Rectangle updateRect = new Rectangle(x, y, width, height);
        if (updateRect.intersects(bounds)) {
            Rectangle updateArea = updateRect.intersection(bounds);
            g.setClip(updateArea);
            g.setColor(getBackground());            
            g.fillRect(updateArea.x, updateArea.y, updateArea.width, updateArea.height);
            g.setColor(getColor());
            g.setFont(getFont());
            FontMetrics fm = g.getFontMetrics();
            int warningWidth = fm.stringWidth(warningString);
            int w_x = (bounds.width - warningWidth)/2;
            int w_y = (bounds.height + fm.getMaxAscent() - fm.getMaxDescent())/2;
            g.drawString(warningString, w_x, w_y);
            g.drawLine(bounds.x, bounds.y+bounds.height-1, bounds.x+bounds.width-1, bounds.y+bounds.height-1);
        }
    }

    String getWarningString() {
        return ownerWindow.getWarningString();
    }

    int getHeight() {
        return defaultHeight; // should implement depending on Font
    }

    Color getBackground() {
        return SystemColor.window;
    }
    Color getColor() {
        return Color.black;
    }
    Font getFont () {
        return ownerWindow.getFont();
    }
    public void repaint() {
        Rectangle bounds = getBounds();
        Graphics g = getGraphics();
        try {
            paint(g, 0, 0, bounds.width, bounds.height);
        } finally {
            g.dispose();
        }
    }
    
    public void handleExposeEvent(int type, long ptr) {        
        super.handleExposeEvent(type, ptr);

        XExposeEvent xe = new XExposeEvent(ptr);
        final int x = xe.get_x();
        final int y = xe.get_y();
        final int width = xe.get_width();
        final int height = xe.get_height();
        EventQueue.invokeLater(new Runnable() {
	    public void run() {	
                Graphics g = getGraphics();
                try {
                    paint(g, x, y, width, height);
                } finally {
                    g.dispose();
                }
            }
        });
    }
    protected boolean isEventDisabled(IXAnyEvent e) {
        return true;
    }
}
