/*
 * @(#)NumberDocument.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.text.BadLocationException;
import javax.swing.text.PlainDocument;
import javax.swing.text.AttributeSet;
import java.awt.Toolkit;

/**
 *
 * @author  mfisher
 * @version 
 */
public class NumberDocument extends PlainDocument {

    public void insertString(int offset, String string,
                             AttributeSet attributes)
                             throws BadLocationException {
        if (isNumeric(string)) {
            super.insertString(offset, string, attributes);
        } else {
            Toolkit.getDefaultToolkit().beep();
        }
    }

    private boolean isNumeric(String s) {
        try {
            Long.valueOf(s);
        } catch (NumberFormatException e) {
            return false;
        }
        return true;
    }

}
