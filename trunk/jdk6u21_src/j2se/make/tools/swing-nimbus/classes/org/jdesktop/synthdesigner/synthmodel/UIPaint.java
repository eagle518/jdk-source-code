/*
 * @(#)UIPaint.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.swingx.designer.paint.Matte;
import org.jdesktop.swingx.designer.paint.PaintModel;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

/**
 * UIPaint
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIPaint extends UIDefault<PaintModel> {

    /** Listener to keep model UiDefaults up to date for this UiPaint */
    private PropertyChangeListener matteListener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            PaintModel paintModel = getValue();
            if (paintModel instanceof Matte) {
                getUiDefaults().put(getName(), ((Matte) paintModel).getColor());
            }
            // propogate the paint change up as PaintModel is a mutable object
            if (evt.getPropertyName().equals("paint")) {
                firePropertyChange("paint", null, getPaint());
                firePropertyChange("value", null, getPaint());
            }
        }
    };

    public UIPaint() {}

    public UIPaint(String id, PaintModel value) {
        super(id, value, (value instanceof Matte) ? ((Matte) value).getUiDefaults() : null);
        // update model defaults
        if (value instanceof Matte) {
            Matte matte = (Matte) value;
            if (getUiDefaults() != null) getUiDefaults().put(getName(), matte.getColor());
            matte.addPropertyChangeListener(matteListener);
        }
    }

    public PaintModel getPaint() {
        return super.getValue();
    }

    public void setPaint(PaintModel c) {
        PaintModel old = getPaint();
        if (old instanceof Matte) old.removePropertyChangeListener(matteListener);
        super.setValue(c);
        firePropertyChange("paint", old, c);
        // update model defaults
        if (c instanceof Matte) {
            Matte matte = (Matte) c;
            getUiDefaults().put(getName(), matte.getColor());
            matte.addPropertyChangeListener(matteListener);
        }
    }
}
