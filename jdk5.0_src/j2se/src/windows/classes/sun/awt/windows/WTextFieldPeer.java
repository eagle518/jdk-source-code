/*
 * @(#)WTextFieldPeer.java	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.im.InputMethodRequests;

class WTextFieldPeer extends WTextComponentPeer implements TextFieldPeer {

    // WComponentPeer overrides
  
    public Dimension getMinimumSize() {
	FontMetrics fm = getFontMetrics(((TextField)target).getFont());
	return new Dimension(fm.stringWidth(getText()) + 24, 
                             fm.getHeight() + 8);
    }

    public void handleEvent(AWTEvent e) {
        int id = e.getID();
        if (id == KeyEvent.KEY_TYPED) {
	    KeyEvent ke = (KeyEvent)e;
	    int keyChar = ke.getKeyChar();
	    if ((keyChar == '\n') && !ke.isAltDown() && !ke.isControlDown()) {
                postEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
                                          getText(), ke.getWhen(),
                                          ke.getModifiers()));
	        return;
	    }
	}
	super.handleEvent(e);
    }

    // TextFieldPeer implementation


    /* This should eventually be a direct native method. */
    public void setEchoChar(char c) {
      	setEchoCharacter(c);
    }

    public Dimension getPreferredSize(int cols) {
	return getMinimumSize(cols);
    }

    public Dimension getMinimumSize(int cols) {
	FontMetrics fm = getFontMetrics(((TextField)target).getFont());
	return new Dimension(fm.charWidth('0') * cols + 24, fm.getHeight() + 8);
    }

    public InputMethodRequests getInputMethodRequests() {
           return null;
    }



    // Toolkit & peer internals

    WTextFieldPeer(TextField target) {
	super(target);
    }

    native void create(WComponentPeer parent);

    void initialize() {
	TextField tf = (TextField)target;
	if (tf.echoCharIsSet()) {
	    setEchoChar(tf.getEchoChar());
	}
	super.initialize();
    }

    // deprecated methods

    /**
     * DEPRECATED but, for now, called by setEchoChar(char).
     */
    public native void setEchoCharacter(char c);

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	return getMinimumSize();
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize(int cols) {
	return getMinimumSize(cols);
    }

    /**
     * DEPRECATED
     */
    public Dimension preferredSize(int cols) {
	return getPreferredSize(cols);
    }

}
