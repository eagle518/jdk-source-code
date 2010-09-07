/*
 * @(#)RadialGradient.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package org.jdesktop.swingx.designer.paint;

import java.awt.Color;
import java.awt.MultipleGradientPaint.CycleMethod;
import java.awt.Paint;
import java.awt.RadialGradientPaint;

/**
 * Represents a RadialGradientPaint.
 *
 * @author rbair
 */
public class RadialGradient extends AbstractGradient {
    protected Paint createPaint(float[] fractions, Matte[] mattes, CycleMethod method) {
        Color[] colors = new Color[mattes.length];
        for (int i = 0; i < colors.length; i++) {
            colors[i] = mattes[i].getColor();
        }
        return new RadialGradientPaint(.5f, .5f, 1, fractions, colors, method);
    }

    @Override public RadialGradient clone() {
        RadialGradient gradient = new RadialGradient();
        copyTo(gradient);
        return gradient;
    }
}
