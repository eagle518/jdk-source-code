/*
 * @(#)UIIconRegion.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

/**
 * A UIRegion subclass which is used for generating icons. For example, JRadioButton and JCheckBox represent themselves
 * mainly via their icons. However, from the designers perspective, the main design isn't an "icon", but just a region
 * on the button.
 * <p/>
 * That type of region is represented by a UIIconRegion. UIIconRegion contains a string which references the UIDefault
 * value associated with this icon. For example, RadioButton.icon.
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIIconRegion extends UIRegion {
    /** The UiDefaults key which this icon should be stored for basic LaF to find it. This is absolute */
    private String basicKey = null;

    public UIIconRegion() {
        super();
    }

    public String getBasicKey() {
        return basicKey;
    }

    public void setBasicKey(String basicKey) {
        String old = getBasicKey();
        this.basicKey = basicKey;
        firePropertyChange("basicKey",old,getBasicKey());
    }
}
