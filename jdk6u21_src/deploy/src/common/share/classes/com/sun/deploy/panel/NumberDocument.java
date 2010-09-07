/*
 * @(#)NumberDocument.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
