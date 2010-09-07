/*
 * @(#)GradientStop.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.paint;

import org.jdesktop.beans.AbstractBean;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

/** Each stop is defined linearly, at positions between 0 and 1. */
public final class GradientStop extends AbstractBean implements Cloneable {
    private float position;
    private Matte color;
    private PropertyChangeListener matteListener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            firePropertyChange("color", null, color);
        }
    };

    /**
     * The midpoint to the right of the stop. Must be 0 &lt;= midpoint &lt;= 1. The midpoint value of the last Stop is
     * ignored.
     */
    private float midpoint;

    public GradientStop() {}

    public GradientStop(float position, Matte color) {
        if (color == null) {
            throw new IllegalArgumentException("Color must not be null");
        }

        this.position = clamp(0, 1, position);
        this.color = color;
        this.midpoint = .5f;

        if (this.color != null) {
            this.color.addPropertyChangeListener("color", matteListener);
        }
    }


    public GradientStop clone() {
        GradientStop clone = new GradientStop(this.position, this.color.clone());
        clone.midpoint = midpoint;
        return clone;
    }

    public final float getPosition() {
        return position;
    }

    public final void setPosition(float position) {
        float old = this.position;
        this.position = clamp(0, 1, position);
        firePropertyChange("position", old, this.position);
    }

    public final Matte getColor() {
        return color;
    }

    public final void setColor(Matte c) {
        if (c == null) throw new IllegalArgumentException("Color must not be null");
        Matte old = this.color;
        if (old != null) old.removePropertyChangeListener(matteListener);
        this.color = c;
        if (this.color != null) this.color.addPropertyChangeListener(matteListener);
        firePropertyChange("color", old, c);
    }

    public final void setOpacity(int opacity) {
        int old = getOpacity();
        color.setAlpha(opacity);
        firePropertyChange("opacity", old, getOpacity());
    }

    public final int getOpacity() {
        return color.getAlpha();
    }

    public final float getMidpoint() {
        return midpoint;
    }

    public final void setMidpoint(float midpoint) {
        float old = this.midpoint;
        this.midpoint = clamp(0, 1, midpoint);
        firePropertyChange("midpoint", old, this.midpoint);
    }

    private float clamp(float lo, float hi, float value) {
        if (value < lo) {
            return lo;
        } else if (value > hi) {
            return hi;
        } else {
            return value;
        }
    }
}
