/*
 * @(#)MCheckboxMenuItemPeer.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;


import java.awt.*;
import java.awt.event.*;
import java.awt.peer.*;

class MCheckboxMenuItemPeer extends MMenuItemPeer 
                            implements CheckboxMenuItemPeer {
    private boolean inUpCall=false;
    private boolean inInit=false;

    native void pSetState(boolean t);
    native boolean getState();

    void create(MMenuPeer parent) {
        super.create(parent);
	inInit=true;
        setState(((CheckboxMenuItem)target).getState());
	inInit=false;
    }

    MCheckboxMenuItemPeer(CheckboxMenuItem target) {
	this.target = target;
	isCheckbox = true;
	MMenuPeer parent = (MMenuPeer) MToolkit.targetToPeer(getParent_NoClientCode(target));
	create(parent);
    }

    public void setState(boolean t) {
        if (!nativeCreated) {
            return;
        }
	if (!inUpCall && (t != getState())) {
	    pSetState(t);
	    if (!inInit) {
	    	// 4135725 : do not notify on programatic changes
		// notifyStateChanged(t);
	    }
	}
    }

    void notifyStateChanged(boolean state) {
	CheckboxMenuItem cb = (CheckboxMenuItem)target;
	ItemEvent e = new ItemEvent(cb, 
			  ItemEvent.ITEM_STATE_CHANGED,
			  cb.getLabel(), 
			  state ? ItemEvent.SELECTED : ItemEvent.DESELECTED);
	postEvent(e);
    }


    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void action(long when, int modifiers, boolean state) {
	final CheckboxMenuItem cb = (CheckboxMenuItem)target;
	final boolean newState = state;

	MToolkit.executeOnEventHandlerThread(cb, new Runnable() {
	    public void run() {
	        cb.setState(newState);
		notifyStateChanged(newState);
	    }
	});
			// Fix for 4024569.
			// Invoke the parent class action method
			// so that action events can be
			// generated.
	super.action(when, modifiers);

    } // action()
} // class MCheckboxMenuItemPeer
	
