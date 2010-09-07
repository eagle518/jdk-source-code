/*
 * @(#)WDialogPeer.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.util.Vector;
import java.awt.*;
import java.awt.peer.*;
import sun.awt.im.*;

class WDialogPeer extends WWindowPeer implements DialogPeer {
    // Toolkit & peer internals
 
    // Platform default background for dialogs.  Gets set on target if
    // target has none explicitly specified.
    final static Color defaultBackground =  SystemColor.control;

    // If target doesn't have its background color set, we set its
    // background to platform default.
    boolean needDefaultBackground;
   
    WDialogPeer(Dialog target) {
	super(target);

	InputMethodManager imm = InputMethodManager.getInstance();
	String menuString = imm.getTriggerMenuString();
	if (menuString != null)
	{ 
	  pSetIMMOption(menuString);
	}
    }

    native void create(WComponentPeer parent);
    native void showModal();
    native void endModal();

    void initialize() {
        Dialog target = (Dialog)this.target;
        // Need to set target's background to default _before_ a call
        // to super.initialize.
        if (needDefaultBackground) {
            target.setBackground(defaultBackground);
        }

        super.initialize();

	if (target.getTitle() != null) {
	    setTitle(target.getTitle());
	}
	setResizable(target.isResizable());
    }

    public void show() {
        focusableWindow = ((Window)target).isFocusableWindow();
	Dialog dlg = (Dialog)target;
	if (dlg.isModal()) {
	    showModal();
	    WToolkit.getWToolkit().notifyModalityChange(ModalityEvent.MODALITY_PUSHED);
            if (((Window)target).isAlwaysOnTop()) {
                setAlwaysOnTop(true);
            }
        } else {
            super.show();
        }
    }

    public void hide() {
	if (((Dialog)target).isModal()) {
	    WToolkit.getWToolkit().notifyModalityChange(ModalityEvent.MODALITY_POPPED);
            endModal();
        } else {
            super.hide();
        }
    }

    public Dimension getMinimumSize() {
        if (((Dialog)target).isUndecorated()) {
            return super.getMinimumSize();
        } else {
            return new Dimension(getSysMinWidth(), getSysMinHeight());
        }
    }

    public void reshape(int x, int y, int width, int height) {
        if (((Dialog)target).isUndecorated()) {
            super.reshape(x,y,width,height);
        } else {
            reshapeFrame(x,y,width,height);
        }
    }

    /* Native create() peeks at target's background and if it's null
     * calls this method to arrage for default background to be set on
     * target.  Can't make the check in Java, since getBackground will
     * return owner's background if target has none set.
     */
    private void setDefaultColor() {
        // Can't call target.setBackground directly, since we are
        // called on toolkit thread.  Can't schedule a Runnable on the
        // EventHandlerThread because of the race condition.  So just
        // set a flag and call target.setBackground in initialize.
        needDefaultBackground = true;
    }

    native void pSetIMMOption(String option);
    void notifyIMMOptionChange(){
      InputMethodManager.getInstance().notifyChangeRequest((Component)target);
    }  
}
