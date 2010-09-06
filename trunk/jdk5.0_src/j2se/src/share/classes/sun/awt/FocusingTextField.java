/*
 * @(#)FocusingTextField.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.*;

/**
 * FocusingTextField: a subclass of java.awt.TextField that handles moving the
 * input focus from field to field, as when the user hits 'return.'
 *
 * @version 1.12, 12/19/03
 * @author Herb Jellinek
 */

public class FocusingTextField extends TextField {

    /** The field to move to on 'return' - can be null. */
    TextField next;

    /** If true, select the contents of the field when it gets the focus. */
    boolean willSelect;

    /**
     * Create a FocusingTextField.
     * @param cols number of columns of text.
     */
    public FocusingTextField(int cols) {
	super("", cols);
    }

    /**
     * Create a FocusingTextField.
     * @param cols number of columns of text.
     * @param willSelect if true, will select all contents of field when
     * focus is gained.
     */
    public FocusingTextField(int cols, boolean willSelect) {
	this(cols);
	this.willSelect = willSelect;
    }
      
    public void setWillSelect(boolean will) {
	willSelect = will;
    }

    public boolean getWillSelect() {
	return willSelect;
    }
    
    /**
     * Call this to set the next field to receive the input focus.
     * @param next the next TextField in order - can be null.
     */
    public void setNextField(TextField next) {
	this.next = next;
    }
    
    /**
     * We got the focus.  If willSelect is true, select everything.
     */
    public boolean gotFocus(Event e, Object arg) {
	if (willSelect) {
	    select(0, getText().length());
	}
	return true;
    }
    
    /**
     * We lost the focus.  If willSelect is true, deselect everything.
     */
    public boolean lostFocus(Event e, Object arg) {
	if (willSelect) {
	    select(0, 0);
	}
	return true;
    }

    /**
     * Pass the focus to the next guy, if any.
     */
    public void nextFocus() {
	if (next != null) {
	    next.requestFocus();
	}
	super.nextFocus();
    }
}

