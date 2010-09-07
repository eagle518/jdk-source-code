/*
 * @(#)WCheckboxMenuItemPeer.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ItemEvent;

class WCheckboxMenuItemPeer extends WMenuItemPeer implements CheckboxMenuItemPeer {

    // CheckboxMenuItemPeer implementation

    public native void setState(boolean t);

    // Toolkit & peer internals

    WCheckboxMenuItemPeer(CheckboxMenuItem target) {
        super(target);
        isCheckbox = true;
        setState(target.getState());
    }

    // native callbacks

    public void handleAction(final boolean state) {
	final CheckboxMenuItem target = (CheckboxMenuItem)this.target;
	WToolkit.executeOnEventHandlerThread(target, new Runnable() {
	    public void run() {
		target.setState(state);
		postEvent(new ItemEvent(target, ItemEvent.ITEM_STATE_CHANGED,
                                        target.getLabel(), (state)
                                          ? ItemEvent.SELECTED
                                          : ItemEvent.DESELECTED));
	    }
        });
    }
}
