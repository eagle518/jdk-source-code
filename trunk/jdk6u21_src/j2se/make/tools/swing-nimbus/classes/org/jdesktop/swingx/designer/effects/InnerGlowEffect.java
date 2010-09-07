/*
 * @(#)InnerGlowEffect.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.effects;

import org.jdesktop.swingx.designer.paint.Matte;

import javax.swing.UIDefaults;
import java.awt.Color;

/**
 * InnerGlowEffect
 *
 * @author Created by Jasper Potts (Jun 21, 2007)
 * @version 1.0
 */
public class InnerGlowEffect extends InnerShadowEffect {

    protected InnerGlowEffect() {
        distance = 0;
    }

    public InnerGlowEffect(UIDefaults uiDefaults) {
        color = new Matte(new Color(255, 255, 211), uiDefaults);
    }

    /**
     * Get the display name for this effect
     *
     * @return The user displayable name
     */
    public String getDisplayName() {
        return "Inner Glow";
    }
}
