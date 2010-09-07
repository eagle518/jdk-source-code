/*
 * @(#)UIInsets.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import java.awt.Insets;

/**
 * UIInsets
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIInsets extends UIDefault<Insets> {

    public UIInsets() {
    }

    public UIInsets(String id, Insets value) {
        super(id, value);
    }

    public Insets getInsets() {
        return super.getValue();
    }

    public void setInsets(Insets i) {
        Insets old = getInsets();
        super.setValue(i);
        firePropertyChange("insets", old, i);
        // update model defaults
        getUiDefaults().put(getName(), i);
    }


    public String toString() {
        return "UiInset(" + getName() + ")" +
                ((getInsets() == null) ? " NONE" : "(" + getInsets().top + "," + getInsets().left + "," +
                        getInsets().bottom + "," + getInsets().right + ")");
    }
}
