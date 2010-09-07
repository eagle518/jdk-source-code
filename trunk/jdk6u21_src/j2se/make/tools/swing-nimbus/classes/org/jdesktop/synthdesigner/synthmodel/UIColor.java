/*
 * @(#)UIColor.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.swingx.designer.paint.Matte;

import javax.swing.UIDefaults;
import java.awt.Color;

/**
 * UIColor
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIColor extends UIPaint {

    public UIColor() {
    }

    public UIColor(String id, Matte value) {
        super(id, value);
    }

    public UIColor(String id, Color color, UIDefaults modelDefaults) {
        this(id, new Matte(color, modelDefaults));
    }

    public Matte getPaint() {
        return (Matte) super.getPaint();
    }

    public void setPaint(Matte c) {
        super.setPaint(c);
    }
}
