/*
 * @(#)PainterBorder.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package org.jdesktop.synthdesigner.synthmodel;

import javax.swing.border.EmptyBorder;

/**
 * Represents a border that refers to a Painter to do it's work. This border
 * doesn't actually render -- it is just used as part of the model.
 * 
 * @author Richard Bair
 */
public class PainterBorder extends EmptyBorder {
    private String painterName;
    public PainterBorder(String painterName, int top, int left, int bottom, int right) {
        super(top, left, bottom, right);
        this.painterName = painterName;
    }
    
    public String getPainterName() { return painterName; }
}
