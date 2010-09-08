/*
 * @(#)WCheckboxPeer.java	1.28 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ItemEvent;

public class WCheckboxPeer extends WComponentPeer implements CheckboxPeer {

    // CheckboxPeer implementation

    public native void setState(boolean state);
    public native void setCheckboxGroup(CheckboxGroup g);
    public native void setLabel(String label);
    
    private static native int getCheckMarkSize();

    public Dimension getMinimumSize() {
	String lbl = ((Checkbox)target).getLabel();
        int marksize = getCheckMarkSize();
        if (lbl == null) {
            lbl = "";
        }
        FontMetrics fm = getFontMetrics(((Checkbox)target).getFont());
        /* 
         * Borders between check mark and text and between text and edge of 
         * checkbox should both be equal to marksize/4, here's where marksize/2
         * goes from. Marksize is currently constant ( = 16 pixels) on win32.
         */
        return new Dimension(fm.stringWidth(lbl) + marksize/2 + marksize,
                             Math.max(fm.getHeight() + 8,  marksize));
    }

    public boolean isFocusable() {
	return true;
    }

    // Toolkit & peer internals

    WCheckboxPeer(Checkbox target) {
	super(target);
    }

    native void create(WComponentPeer parent);

    void initialize() {
	Checkbox t = (Checkbox)target;
	setState(t.getState());
	setCheckboxGroup(t.getCheckboxGroup());

	Color bg = ((Component)target).getBackground();
	if (bg != null) {
	    setBackground(bg);
	}

	super.initialize();
    }

    public boolean shouldClearRectBeforePaint() {
        return false;
    }
    
    // native callbacks

    void handleAction(final boolean state) {
	final Checkbox cb = (Checkbox)this.target;
	WToolkit.executeOnEventHandlerThread(cb, new Runnable() {
	    public void run() {
                CheckboxGroup chg = cb.getCheckboxGroup();
                if ((chg != null) && (cb == chg.getSelectedCheckbox()) && cb.getState()) {
                    return;
                }
		cb.setState(state);
		postEvent(new ItemEvent(cb, ItemEvent.ITEM_STATE_CHANGED,
                                cb.getLabel(),
                                state? ItemEvent.SELECTED : ItemEvent.DESELECTED));
	    }
	});
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	    return getMinimumSize();
    }

}
