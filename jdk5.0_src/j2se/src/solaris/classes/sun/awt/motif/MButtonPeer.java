/*
 * @(#)MButtonPeer.java	1.32 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ActionEvent;

class MButtonPeer extends MComponentPeer implements ButtonPeer {
    native void create(MComponentPeer peer);
    public native void setLabel(String label);
    
    MButtonPeer(Button target) {
	super(target);
    }

    public Dimension getMinimumSize() {
	FontMetrics fm = getFontMetrics(target.getFont());
	String label = ((Button)target).getLabel();
	if ( label == null ) {
	    label = "";
	}
	return new Dimension(fm.stringWidth(label) + 14, 
			     fm.getHeight() + 8);
    }

    public boolean isFocusable() {
	return true;
    }

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void action(final long when, final int modifiers) {
	MToolkit.executeOnEventHandlerThread(target, new Runnable() {
	    public void run() {
                postEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
                                          ((Button)target).getActionCommand(),
                                          when, modifiers));
	    }
	});
    }

    /*
     * Print the native component by rendering the Motif look ourselves.
     * ToDo(aim): needs to query native motif for more accurate size and
     * color information.
     */
    public void print(Graphics g) {
        Button b = (Button)target;
	Dimension d = b.size();
	Color bg = b.getBackground();
	Color fg = b.getForeground();

	g.setColor(bg);
	g.fillRect(2, 2, d.width - 3, d.height - 3);
	draw3DRect(g, bg, 1, 1, d.width - 2, d.height - 2, true);

	g.setColor(fg);
	g.setFont(b.getFont());
	FontMetrics fm = g.getFontMetrics();
	String lbl = b.getLabel();
	g.drawString(lbl, (d.width - fm.stringWidth(lbl)) / 2,
		          (d.height + fm.getMaxAscent() - fm.getMaxDescent()) / 2);

	target.print(g);
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	return getMinimumSize();
    }

}
