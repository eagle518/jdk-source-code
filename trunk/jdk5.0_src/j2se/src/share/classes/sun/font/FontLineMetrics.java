/*
 * @(#)FontLineMetrics.java	1.2 03/08/04
 *
 * (C) Copyright IBM Corp. 2003, All Rights Reserved
 *
 */

package sun.font;

import java.awt.font.FontRenderContext;
import java.awt.font.LineMetrics;

/**
 * Metrics from a font for layout of characters along a line
 * and layout of set of lines.
 * This and CoreMetrics replace what was previously a private internal class of Font
 */
public final class FontLineMetrics extends LineMetrics implements Cloneable {
    public int numchars; // mutated by Font
    public final CoreMetrics cm;
    public final FontRenderContext frc;

    public FontLineMetrics(int numchars, CoreMetrics cm, FontRenderContext frc) {
        this.numchars = numchars;
        this.cm = cm;
        this.frc = frc;
    }

    public final int getNumChars() {
        return numchars;
    }

    public final float getAscent() {
        return cm.ascent;
    }

    public final float getDescent() {
        return cm.descent;
    }

    public final float getLeading() {
        return cm.leading;
    }

    public final float getHeight() {
        return cm.height;
    }

    public final int getBaselineIndex() {
        return cm.baselineIndex;
    }

    public final float[] getBaselineOffsets() {
        return (float[])cm.baselineOffsets.clone();
    }

    public final float getStrikethroughOffset() {
        return cm.strikethroughOffset;
    }

    public final float getStrikethroughThickness() {
        return cm.strikethroughThickness;
    }

    public final float getUnderlineOffset() {
        return cm.underlineOffset;
    }

    public final float getUnderlineThickness() {
        return cm.underlineThickness;
    }

    public final int hashCode() {
	return cm.hashCode();
    }

    public final boolean equals(Object rhs) {
        try {
            return cm.equals(((FontLineMetrics)rhs).cm);
        }
        catch (ClassCastException e) {
            return false;
        }
    }

    public final Object clone() {
        // frc, cm do not need deep clone
        try {
            return super.clone();
        }
        catch (CloneNotSupportedException e) {
            throw new InternalError();
        }
    }
}

