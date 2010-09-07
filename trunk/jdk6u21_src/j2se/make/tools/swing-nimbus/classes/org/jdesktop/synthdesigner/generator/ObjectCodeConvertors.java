/*
 * @(#)ObjectCodeConvertors.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.generator;

import java.awt.*;

/**
 * ObjectCodeConvertors
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class ObjectCodeConvertors {
    static java.math.MathContext ctx = new java.math.MathContext(3);

    /**
     * Given a value (x), encode it such that 0 -> 1 is to the left of a, 1 -> 2 is between a and b, and 2 -> 3
     * is to the right of b.
     * 
     * @param w width in the case of the x axis, height in the case of the y axis.
     */
    static float encode(float x, float a, float b, float w) {
        float r = 0;
        if (x < a) {
            r = (x / a);
        } else if (x > b) {
            r = 2 + ((x - b) / (w - b));
        } else if (x == a && x == b) {
            return 1.5f;
        } else {
            r = 1 + ((x - a) / (b - a));
        }

        if (Float.isNaN(r)) {
            System.err.println("[Error] Encountered NaN: encode(" + x + ", " + a + ", " + b + ", " + w + ")");
            return 0;
        } else if (Float.isInfinite(r)) {
            System.err.println("[Error] Encountered Infinity: encode(" + x + ", " + a + ", " + b + ", " + w + ")");
            return 0;
        } else if (r < 0) {
            System.err.println("[Error] encoded value was less than 0: encode(" + x + ", " + a + ", " + b + ", " + w + ")");
            return 0;
        } else if (r > 3) {
            System.err.println("[Error] encoded value was greater than 3: encode(" + x + ", " + a + ", " + b + ", " + w + ")");
            return 3;
        } else {
            //for prettyness sake (and since we aren't really going to miss
            //any accuracy here) I'm rounding this to 3 decimal places
//                return java.math.BigDecimal.valueOf(r).round(ctx).doubleValue();
            return r;
        }
    }
    
    static String convert(Paint paint) {
        //TODO need to support writing out other Paints, such as gradients
        if (paint instanceof Color) {
            return convert((Color) paint);
        } else {
            System.err.println("[WARNING] Unable to encode a paint in the encode(Paint) method: " + paint);
            return "null";
        }
    }

    /**
     * Given a Color, write out the java code required to create a new Color.
     *
     * @param color The color to convert
     * @return String of the code for the color
     */
    static String convert(Color color) {
        return "new Color(" +
                color.getRed() + ", " +
                color.getGreen() + ", " +
                color.getBlue() + ", " +
                color.getAlpha() + ")";
    }

    static String convert(Insets i) {
        return "new Insets(" + i.top + ", " + i.left + ", " + i.bottom + ", " + i.right + ")";
    }

    static String convert(Dimension d) {
        return "new Dimension(" + d.width + ", " + d.height + ")";
    }

}
