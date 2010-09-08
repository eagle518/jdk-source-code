/*
 * @(#)WButtonPeer.java	1.27 03/12/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

class WButtonPeer extends WComponentPeer implements ButtonPeer {

    static {
        initIDs();
    }

    // ComponentPeer overrides

    public Dimension getMinimumSize() {
	FontMetrics fm = getFontMetrics(((Button)target).getFont());
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

    // ButtonPeer implementation

    public native void setLabel(String label);

    // Toolkit & peer internals

    WButtonPeer(Button target) {
	super(target);
    }

    native void create(WComponentPeer peer);

    // native callbacks

    // NOTE: This is called on the privileged toolkit thread. Do not
    //       call directly into user code using this thread!
    public void handleAction(final long when, final int modifiers) {
        // Fixed 5064013: the InvocationEvent time should be equals
        // the time of the ActionEvent
	WToolkit.executeOnEventHandlerThread(target, new Runnable() {
	    public void run() {
                postEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
                                          ((Button)target).getActionCommand(),
                                          when, modifiers));
	    }
	}, when);
    }


    public boolean shouldClearRectBeforePaint() {
        return false;
    }
    
    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	return getMinimumSize();
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();
    
    public boolean handleJavaKeyEvent(KeyEvent e) {
         switch (e.getID()) {
            case KeyEvent.KEY_RELEASED:
                if (e.getKeyCode() == KeyEvent.VK_SPACE){
                    handleAction(e.getWhen(), e.getModifiers());
                }
            break;
         }
         return false;
    }
}
