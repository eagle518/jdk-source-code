/*
 * @(#)UIFont.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import java.awt.Font;
import org.jdesktop.swingx.designer.font.Typeface;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import javax.swing.UIDefaults;

/**
 * Represents a single font entry in the UIDefaults table. Each UIFont takes a
 * list of Typefaces. These typefaces are listed by order of preference. Thus,
 * when putting a font into UIDefaults, the code can check whether each font
 * exists, and when it finds the first font that does, insert it.
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIFont extends UIDefault<List<Typeface>> implements Cloneable {
    
    private void updateUIDefaults() {
        if (getUiDefaults() != null) {
            for (Typeface t : getFonts()) {
                if (t.isFontSupported()) {
                    getUiDefaults().put(getName(), t.getFont());
                    return;
                }
            }
        }
        
        //TODO must not have found any. Default to the Default platform font
        getUiDefaults().put(getName(), new Font("Arial", Font.PLAIN, 12));
    }
    
    public UIFont() {
        setValue(new ArrayList<Typeface>());
    }

    public UIFont(String id, List<Typeface> values, UIDefaults defaults) {
        super(id, values, defaults);
        updateUIDefaults();
    }

    public UIFont(String id, Font font, UIDefaults modelDefaults) {
        this(id, Arrays.asList(new Typeface(font, modelDefaults)), modelDefaults);
    }

    public List<Typeface> getFonts() {
        return super.getValue();
    }
    
    private void setFonts(List<Typeface> values) {
        super.setValue(values);
    }
}
