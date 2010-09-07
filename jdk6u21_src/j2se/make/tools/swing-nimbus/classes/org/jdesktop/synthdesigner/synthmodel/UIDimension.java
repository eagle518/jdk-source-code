/*
 * @(#)UIDimension.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import java.awt.Dimension;

/**
 * UIDimension
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIDimension extends UIDefault<Dimension> {
    public UIDimension() {
    }
    
    public Dimension getDimension() {
        return super.getValue();
    }
    
    public void setDimension(Dimension d) {
        Dimension old = getDimension();
        super.setValue(d);
        firePropertyChange("dimension", old, d);
    }
}
