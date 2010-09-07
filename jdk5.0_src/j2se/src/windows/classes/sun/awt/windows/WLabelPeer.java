/*
 * @(#)WLabelPeer.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

class WLabelPeer extends WComponentPeer implements LabelPeer {

    // ComponentPeer overrides

    public Dimension getMinimumSize() {
	FontMetrics fm = getFontMetrics(((Label)target).getFont());
	String label = ((Label)target).getText();
	if (label == null)
            label = "";
	return new Dimension(fm.stringWidth(label) + 14, fm.getHeight() + 8);
    }

    native void lazyPaint();
    synchronized void start() {
        super.start();
        // if need then paint label
        lazyPaint();
    }
    // LabelPeer implementation

    public boolean shouldClearRectBeforePaint() {
        return false;
    }
    
    public native void setText(String label);
    public native void setAlignment(int alignment);

    // Toolkit & peer internals

    WLabelPeer(Label target) {
	super(target);
    }

    native void create(WComponentPeer parent);

    void initialize() {
	Label	l = (Label)target;

        String  txt = l.getText();
	if (txt != null) {
	    setText(txt);
	}

        int align = l.getAlignment();
	if (align != Label.LEFT) {
	    setAlignment(align);
	}

	Color bg = ((Component)target).getBackground();
	if (bg != null) {
	    setBackground(bg);
	}

	super.initialize();
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	    return getMinimumSize();
    }

}
