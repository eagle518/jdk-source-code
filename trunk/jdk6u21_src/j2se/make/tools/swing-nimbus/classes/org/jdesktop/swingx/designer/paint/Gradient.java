/*
 * @(#)Gradient.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package org.jdesktop.swingx.designer.paint;

import java.awt.Color;
import java.awt.LinearGradientPaint;
import java.awt.MultipleGradientPaint.CycleMethod;
import java.awt.Paint;

/**
 * Represents a GradientPaint or LinearGradientPaint.
 *
 * @author rbair
 */
public class Gradient extends AbstractGradient implements Cloneable {
    protected Paint createPaint(float[] fractions, Matte[] mattes, CycleMethod method) {
        Color[] colors = new Color[mattes.length];
        for (int i = 0; i < colors.length; i++) {
            colors[i] = mattes[i].getColor();
        }
        return new LinearGradientPaint(0, 0, 1, 0, fractions, colors, method);
    }

    @Override public Gradient clone() {
        Gradient gradient = new Gradient();
        copyTo(gradient);
        return gradient;
    }
}
