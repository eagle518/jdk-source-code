/*
 * @(#)ShadowEffect.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.effects;

import org.jdesktop.swingx.designer.BlendingMode;
import org.jdesktop.swingx.designer.paint.Matte;

import javax.swing.UIDefaults;
import java.awt.Color;

/**
 * ShadowEffect - base class with all the standard properties for shadow effects
 *
 * @author Created by Jasper Potts (Jun 18, 2007)
 * @version 1.0
 */
public abstract class ShadowEffect extends Effect {
    protected Matte color;
    protected BlendingMode blendingMode = BlendingMode.NORMAL;
    /** Opacity a float 0-1 for percentage */
    protected float opacity = 0.75f;
    /** Angle in degrees between 0-360 */
    protected int angle = 135;
    /** Distance in pixels */
    protected int distance = 5;
    /** The shadow spread between 0-100 % */
    protected int spread = 0;
    /** Size in pixels */
    protected int size = 5;

    protected ShadowEffect() {}

    ;

    public ShadowEffect(UIDefaults uiDefaults) {
        color = new Matte(Color.BLACK, uiDefaults);
    }

    // =================================================================================================================
    // Bean methods

    public Matte getColor() {
        return color;
    }

    public void setColor(Matte color) {
        Matte old = getColor();
        this.color = color;
        firePropertyChange("color", old, getColor());
    }

    public BlendingMode getBlendingMode() {
        return blendingMode;
    }

    public void setBlendingMode(BlendingMode blendingMode) {
        BlendingMode old = getBlendingMode();
        this.blendingMode = blendingMode;
        firePropertyChange("blendingMode", old, getBlendingMode());
    }

    public float getOpacity() {
        return opacity;
    }

    public void setOpacity(float opacity) {
        float old = getOpacity();
        this.opacity = opacity;
        firePropertyChange("opacity", old, getOpacity());
    }

    public int getAngle() {
        return angle;
    }

    public void setAngle(int angle) {
        int old = getAngle();
        this.angle = angle;
        firePropertyChange("angle", old, getAngle());
    }

    public int getDistance() {
        return distance;
    }

    public void setDistance(int distance) {
        int old = getDistance();
        this.distance = distance;
        firePropertyChange("distance", old, getDistance());
    }

    public int getSpread() {
        return spread;
    }

    public void setSpread(int spread) {
        int old = getSpread();
        this.spread = spread;
        firePropertyChange("spread", old, getSpread());
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        int old = getSize();
        this.size = size;
        firePropertyChange("size", old, getSize());
    }
}
