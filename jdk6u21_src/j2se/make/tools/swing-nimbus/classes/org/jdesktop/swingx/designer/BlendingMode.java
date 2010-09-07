/*
 * @(#)BlendingMode.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.swingx.graphics.BlendComposite;

import javax.swing.DefaultListCellRenderer;
import javax.swing.JList;
import javax.swing.JSeparator;
import javax.swing.ListCellRenderer;
import java.awt.AlphaComposite;
import java.awt.Component;
import java.awt.Composite;

/**
 * BlendingMode - Enum of composite blending modes, setup to match photoshop as closely as possible
 *
 * @author Created by Jasper Potts (May 31, 2007)
 * @version 1.0
 */
public enum BlendingMode {
    NORMAL,
    // DISSOLVE, missing
    // -----------------------------
    DARKEN,
    MULTIPLY,
    COLOR_BURN,
    LINEAR_BURN, // (SUBTRACT)
    // -----------------------------
    LIGHTEN,
    SCREEN,
    COLOR_DODGE,
    LINEAR_DODGE, // (ADD)
    // -----------------------------
    OVERLAY,
    SOFT_LIGHT,
    HARD_LIGHT,
    VIVID_LIGHT, // (HEAT) is close
    LINEAR_LIGHT, // (GLOW) is close
    //PIN_LIGHT, missing
    //HARD_MIX, missing
    // -----------------------------
    DIFFERENCE,
    EXCLUSION,
    // -----------------------------
    HUE, // nowhere close
    SATURATION,
    COLOR,
    LUMINOSITY, // close but not exact
    //LIGHTER_COLOR, missing
    //DARKER_COLOR, missing
    ;

    public Composite getComposite(float alpha) {
        switch (this) {
            case NORMAL:
            default:
                return AlphaComposite.getInstance(AlphaComposite.SRC_OVER, alpha);
            case DARKEN:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.DARKEN, alpha);
            case MULTIPLY:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.MULTIPLY, alpha);
            case COLOR_BURN:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.COLOR_BURN, alpha);
            case LINEAR_BURN:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.SUBTRACT, alpha);
            case LIGHTEN:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.LIGHTEN, alpha);
            case SCREEN:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.SCREEN, alpha);
            case COLOR_DODGE:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.COLOR_DODGE, alpha);
            case LINEAR_DODGE:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.ADD, alpha);
            case OVERLAY:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.OVERLAY, alpha);
            case SOFT_LIGHT:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.SOFT_LIGHT, alpha);
            case HARD_LIGHT:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.HARD_LIGHT, alpha);
            case VIVID_LIGHT:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.HEAT, alpha);
            case LINEAR_LIGHT:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.GLOW, alpha);
            case DIFFERENCE:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.DIFFERENCE, alpha);
            case EXCLUSION:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.EXCLUSION, alpha);
            case HUE:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.HUE, alpha);
            case SATURATION:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.SATURATION, alpha);
            case COLOR:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.COLOR, alpha);
            case LUMINOSITY:
                return BlendComposite.getInstance(BlendComposite.BlendingMode.LUMINOSITY, alpha);
        }
    }

    // =================================================================================================================
    // Helper methods for creating Blending Mode Combo Box

    public static final Object[] BLENDING_MODES = new Object[]{
            BlendingMode.NORMAL,
            // DISSOLVE, missing
            "-",
            BlendingMode.DARKEN,
            BlendingMode.MULTIPLY,
            BlendingMode.COLOR_BURN,
            BlendingMode.LINEAR_BURN, // (SUBTRACT)
            "-",
            BlendingMode.LIGHTEN,
            BlendingMode.SCREEN,
            BlendingMode.COLOR_DODGE,
            BlendingMode.LINEAR_DODGE, // (ADD)
            "-",
            BlendingMode.OVERLAY,
            BlendingMode.SOFT_LIGHT,
            BlendingMode.HARD_LIGHT,
            BlendingMode.VIVID_LIGHT, // (HEAT) is close
            BlendingMode.LINEAR_LIGHT, // (GLOW) is close
            //PIN_LIGHT, missing
            //HARD_MIX, missing
            "-",
            BlendingMode.DIFFERENCE,
            BlendingMode.EXCLUSION,
            "-",
            BlendingMode.HUE, // nowhere close
            BlendingMode.SATURATION,
            BlendingMode.COLOR,
            BlendingMode.LUMINOSITY, // close but not exact
    };

/*    public static final ListCellRenderer BLEND_MODE_COMBOBOX_RENDERER = new DefaultListCellRenderer() {
        private final JSeparator separator = new JSeparator();

        @Override public Component getListCellRendererComponent(JList jList, Object object, int i, boolean b,
                                                                boolean b1) {
            String text = ((object != null)) ? object.toString() : "";
            if (text.equals("-")) {
                return separator;
            } else {
                return super.getListCellRendererComponent(jList, text.toLowerCase().replace('_', ' '), i, b, b1);
            }
        }
    };
*/
}
