/*
 * @(#)UIBorder.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import javax.swing.border.Border;

/**
 * UIBorder
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIBorder extends UIDefault<Border> {

    public UIBorder() {
    }

    public UIBorder(String id, Border b) {
        super(id, b);
    }
    
    public Border getBorder() {
        return super.getValue();
    }
    
    public void setBorder(Border b) {
        Border old = getBorder();
        super.setValue(b);
        firePropertyChange("border", old, b);
    }
}
