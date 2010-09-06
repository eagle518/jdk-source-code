/*
 * @(#)SimpleInputMethodWindow.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.im;

import java.awt.Frame;

/**
 * Implements a simple input method window that provides the minimal
 * functionality as specified in
 * {@link java.awt.im.spi.InputMethodContext#createInputMethodWindow}.
 *
 * @version 1.10 12/19/03
 */
public class SimpleInputMethodWindow
        extends Frame
        implements InputMethodWindow {

    InputContext inputContext = null;

    /**
     * Constructs a simple input method window.
     */
    public SimpleInputMethodWindow(String title, InputContext context) {
        super(title);
        if (context != null) {
            this.inputContext = context;
        }
	setFocusableWindowState(false);
    }
    
    public void setInputContext(InputContext inputContext) {
        this.inputContext = inputContext;
    }

    public java.awt.im.InputContext getInputContext() {
        if (inputContext != null) {
            return inputContext;
        } else {
            return super.getInputContext();
        }
    }
}
