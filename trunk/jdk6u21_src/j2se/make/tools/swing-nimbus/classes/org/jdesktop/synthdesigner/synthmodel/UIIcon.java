/*
 * @(#)UIIcon.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import javax.swing.Icon;

/**
 * UIIcon
 * 
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIIcon extends UIDefault<Icon> {

    public UIIcon() {
    }

    public Icon getIcon() {
        return super.getValue();
    }
    
    public void setIcon(Icon i) {
        Icon old = getIcon();
        super.setValue(i);
        firePropertyChange("icon", old, i);
    }
}
