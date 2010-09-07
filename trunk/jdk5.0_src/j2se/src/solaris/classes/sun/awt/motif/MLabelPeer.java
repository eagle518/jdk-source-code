/*
 * @(#)MLabelPeer.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.*;
import java.awt.peer.*;

class MLabelPeer extends MComponentPeer implements LabelPeer {
    native void create(MComponentPeer parent);

    public void initialize() {
	Label	l = (Label)target;
	String  txt;
	int	align;

	if ((txt = l.getText()) != null) {
	    setText(l.getText());
	}
	if ((align = l.getAlignment()) != Label.LEFT) {
	    setAlignment(align);
	}
	super.initialize();
    }

    MLabelPeer(Label target) {
	super(target);
    }

    public Dimension getMinimumSize() {
	FontMetrics fm = getFontMetrics(target.getFont());
	String label = ((Label)target).getText();
	if (label == null) label = "";
	return new Dimension(fm.stringWidth(label) + 14, 
			     fm.getHeight() + 8);
    }

    public native void setText(String label);
    public native void setAlignment(int alignment);

    /*
     * Print the native component by rendering the Motif look ourselves.
     */
    public void print(Graphics g) {
        Label l = (Label)target;
	Dimension d = l.size();
	Color bg = l.getBackground();
	Color fg = l.getForeground();

	g.setColor(bg);
	g.fillRect(1, 1, d.width - 2, d.height - 2);

	g.setColor(fg);
	g.setFont(l.getFont());
	FontMetrics fm = g.getFontMetrics();
	String lbl = l.getText();

	switch (l.getAlignment()) {
	  case Label.LEFT:
	    g.drawString(lbl, 2,
			 (d.height + fm.getMaxAscent() - fm.getMaxDescent()) / 2);
	    break;
	  case Label.RIGHT:
	    g.drawString(lbl, d.width - (fm.stringWidth(lbl) + 2), 
			 (d.height + fm.getMaxAscent() - fm.getMaxDescent()) / 2);
	    break;
	  case Label.CENTER:
	    g.drawString(lbl, (d.width - fm.stringWidth(lbl)) / 2, 
			 (d.height + fm.getMaxAscent() - fm.getMaxDescent()) / 2);
	    break;
	}

	target.print(g);
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	    return getMinimumSize();
    }

}
