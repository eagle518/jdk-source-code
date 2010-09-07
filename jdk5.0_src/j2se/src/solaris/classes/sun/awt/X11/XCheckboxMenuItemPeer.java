/*
 * @(#)XCheckboxMenuItemPeer.java	1.9 04/05/31
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;


import sun.awt.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.peer.*;
import java.awt.image.BufferedImage;
import java.awt.geom.Rectangle2D;
import java.awt.geom.AffineTransform;

class XCheckboxMenuItemPeer extends XMenuItemPeer 
                            implements CheckboxMenuItemPeer {

    private boolean inUpCall=false;
    private boolean state=false;

    // Duplicate code from XCheckboxMenuItemPeer.
    private static final double MASTER_SIZE = 128.0;
    private static final Polygon MASTER_CHECKMARK = new Polygon(
        new int[] {1, 25,56,124,124,85, 64},  // X-coords
        new int[] {59,35,67,  0, 12,66,123},  // Y-coords
      7);

    boolean getState() {
        return state;
    }

    XCheckboxMenuItemPeer(CheckboxMenuItem target) {
	super(target);
        setState(((CheckboxMenuItem)target).getState());
    }

    public void setState(boolean t) {
        if (state != t) {
            state = t;
            XMenuPeer menuPeer = getMenuPeer();
            if ((menuPeer != null) && (menuPeer.isVisible())) {
                menuPeer.repaintMenuItem(this);
            }
        }
    }

    void notifyStateChanged(boolean state) {
	CheckboxMenuItem cb = (CheckboxMenuItem)menuTarget;
	ItemEvent e = new ItemEvent(cb, 
			  ItemEvent.ITEM_STATE_CHANGED,
			  getLabel(), 
			  getState() ? ItemEvent.SELECTED : ItemEvent.DESELECTED);
	postEvent(e);
    }


    public void action(long when, int modifiers, boolean state) {
        final boolean newState = state;
	XToolkit.executeOnEventHandlerThread((CheckboxMenuItem)menuTarget, new Runnable() {
	    public void run() {
	        ((CheckboxMenuItem)menuTarget).setState(newState);
		notifyStateChanged(newState);
	    }
	});
	// Fix for 4024569.
	// Invoke the parent class action method
	// so that action events can be
	// generated.
//	super.action(when, modifiers);
    } // action()

    void paintCheck(Graphics g, int x, int y, int width, int height) {
        boolean useBufferedImage = false;
        Graphics2D g2 = null;
        BufferedImage buffer = null;
        int rx = x;
        int ry = y;
        if (!(g instanceof Graphics2D)) {
            // Fix for 5045936, 5055171. While printing, g is an instance 
            //   of sun.print.ProxyPrintGraphics which extends Graphics.
            //   So we use a separate buffered image and its graphics is
            //   always Graphics2D instance
            buffer = graphicsConfig.createCompatibleImage(width, height);
            g2 = buffer.createGraphics();
            useBufferedImage = true;
            rx = 0;
            ry = 0;
        }
        else {
            g2 = (Graphics2D)g;
        }
        try {
            // Paint shadow
            g2.setColor(getState() ? getSelect() : getBackground());
            int cbSize = (height * 76 / 100) - 1;
            int boxTop = ry + height/2;
            g2.fillRect(XMenuPeer.checkBorder, boxTop - cbSize/2, cbSize, cbSize);
            draw3DRect(g2, XMenuPeer.checkBorder, boxTop - cbSize/2, cbSize, cbSize, !getState());

            if (getState()) {
                double fsize = (double) cbSize;
                Shape myCheckMark = AffineTransform.getScaleInstance(fsize / MASTER_SIZE, fsize / MASTER_SIZE).createTransformedShape(MASTER_CHECKMARK);

                // Paint check mark
                g2.setColor(getForeground());
                AffineTransform af = g2.getTransform();
                g2.setTransform(AffineTransform.getTranslateInstance((double)XMenuPeer.checkBorder, (double)(boxTop-cbSize / 2)));
                g2.fill(myCheckMark);
                g2.setTransform(af);
            }
        } finally {
            if (useBufferedImage) {
                g2.dispose();
            }
        }
        if (useBufferedImage) {
            g.drawImage(buffer, x, y, null);
        }
    }
} // class XCheckboxMenuItemPeer
