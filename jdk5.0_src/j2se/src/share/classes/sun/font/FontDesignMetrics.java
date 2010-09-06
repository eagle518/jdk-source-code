/*
 * @(#)FontDesignMetrics.java	1.56 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.FontMetrics;
import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.font.FontRenderContext;
import java.awt.font.TextLayout;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

/*
 * This class provides a summary of the glyph measurements  for a Font
 * and a set of hints that guide their display.  It provides more metrics
 * information for the Font than the java.awt.FontMetrics class. There
 * is also some redundancy with that class.
 * <p>
 * The design metrics for a Font are obtained from Font.getDesignMetrics().
 * The FontDesignMetrics object returned will be independent of the
 * point size of the Font.
 * Most users are familiar with the idea of using <i>point size</i> to
 * specify the size of glyphs in a font. This point size defines a
 * measurement between the baseline of one line to the baseline of the
 * following line in a single spaced text document. The point size is
 * based on <i>typographic points</i>, approximately 1/72 of an inch.
 * <p>
 * The Java2D API adopts the convention that one point is equivalent
 * to one unit in user coordinates.  When using a normalized transform
 * for converting user space coordinates to device space coordinates (see
 * GraphicsConfiguration.getDefaultTransform() and
 * GraphicsConfiguration.getNormalizingTransform()), 72 user space units
 * equal 1 inch in device space.  In this case one point is 1/72 of an inch.
 * <p>
 * The FontDesignMetrics class expresses font metrics in terms of arbitrary
 * <i>typographic units</i> (not points) chosen by the font supplier
 * and used in the underlying platform font representations.  These units are
 * defined by dividing the em-square into a grid.  The em-sqaure is the
 * theoretical square whose dimensions are the full body height of the
 * font.  A typographic unit is the smallest measurable unit in the
 * em-square.  The number of units-per-em is determined by the font
 * designer.  The greater the units-per-em, the greater the precision
 * in metrics.  For example, Type 1 fonts divide the em-square into a
 * 1000 x 1000 grid, while TrueType fonts typically use a 2048 x 2048
 * grid.  The scale of these units can be obtained by calling
 * getUnitsPerEm().
 * <p>
 * Typographic units are relative -- their absolute size changes as the
 * size of the of the em-square changes.  An em-square is 9 points high
 * in a 9-point font.  Because typographic units are relative to the
 * em-square, a given location on a glyph will have the same coordinates
 * in typographic units regardless of the point size.
 * <p>
 * Converting typographic units to pixels requires computing pixels-per-em
 * (ppem).  This can be computed as:
 * <pre>
         ppem = device_resolution * (inches-per-point) * pointSize
 * </pre>
 * where device resolution could be measured in pixels/inch and the point
 * size of a font is effectively points/em.  Using a normalized transform
 * from user space to device space (see above), results in 1/72 inch/point.
 * In this case, ppem is equal to the point size on a 72 dpi monitor, so
 * that an N point font displays N pixels high.  In general,
 * <pre>
        pixel_units = typographic_units * (ppem / units_per_em)
 * </pre>
 * @see java.awt.Font
 * @see java.awt.GraphicsConfiguration#getDefaultTransform
 * @see java.awt.GraphicsConfiguration#getNormalizingTransform
 */

public final class FontDesignMetrics extends FontMetrics {

    static final long serialVersionUID = 4480069578560887773L;

    private static final float UNKNOWN_WIDTH = -1;
    private static final int CURRENT_VERSION = 1;

    // height, ascent, descent, leading are reported to the client
    // as an integer this value is added to the true fp value to
    // obtain a value which is usually going to result in a round up
    // to the next integer except for very marginal cases.
    private static float roundingUpValue = 0.95f;

    // These fields are all part of the old serialization representation
    private Font  font;
    private float ascent;
    private float descent;
    private float leading;
    private float maxAdvance;
    private double[] matrix;
    private int[] cache; // now unused, still here only for serialization
    // End legacy serialization fields

    private int serVersion = 0;  // If 1 in readObject, these fields are on the input stream:
    private boolean isAntiAliased;
    private boolean usesFractionalMetrics;
    private AffineTransform frcTx;

    private transient float[] advCache; // transient since values could change across runtimes
    private transient int height = -1;

    private transient FontRenderContext frc;

    private transient double[] devmatrix = null;

    private transient FontStrike fontStrike;

    private static FontRenderContext DEFAULT_FRC = null;

    private static FontRenderContext createDefaultFrc() {

        if (DEFAULT_FRC == null) {
            AffineTransform tx;
            if (GraphicsEnvironment.isHeadless()) {
                tx = new AffineTransform();
            } else {
                tx =  GraphicsEnvironment
                    .getLocalGraphicsEnvironment()
                    .getDefaultScreenDevice()
                    .getDefaultConfiguration()
                    .getDefaultTransform();
            }
            DEFAULT_FRC = new FontRenderContext(tx, false, false);
        }
        return DEFAULT_FRC;
    }

  /*
   * Constructs a new FontDesignMetrics object for the given Font.
   * @param font a Font object.
   */

    public FontDesignMetrics(Font font) {

        this(font, createDefaultFrc());
    }

    public FontDesignMetrics(Font font,
                             FontRenderContext frc) {
      super(font);
      this.font = font;
      this.frc = frc;

      this.isAntiAliased = frc.isAntiAliased();
      this.usesFractionalMetrics = frc.usesFractionalMetrics();

      frcTx = frc.getTransform();

      matrix = new double[4];
      initMatrixAndMetrics();

      initAdvCache();
    }

    private void initMatrixAndMetrics() {

	Font2D font2D = FontManager.getFont2D(font);
        fontStrike = font2D.getStrike(font, frc);
	StrikeMetrics metrics = fontStrike.getFontMetrics();
        this.ascent = metrics.getAscent();
        this.descent = metrics.getDescent();
        this.leading = metrics.getLeading();
        this.maxAdvance = metrics.getMaxAdvance();

	devmatrix = new double[4];
	frcTx.getMatrix(devmatrix);
    }

    private void initAdvCache() {
        advCache = new float[256];
        // 0 is a valid metric so force it to -1
        for (int i = 0; i < 256; i++) {
            advCache[i] = UNKNOWN_WIDTH;
        }
    }

    private void readObject(ObjectInputStream in) throws IOException, 
                                                  ClassNotFoundException {
        
        in.defaultReadObject();
        if (serVersion != CURRENT_VERSION) {
            frc = createDefaultFrc();
            isAntiAliased = frc.isAntiAliased();
            usesFractionalMetrics = frc.usesFractionalMetrics();
            frcTx = frc.getTransform();
        }
        else {
            frc = new FontRenderContext(frcTx, isAntiAliased, usesFractionalMetrics);
        }
        
	// when deserialized, members are set to their default values for their type--
	// not to the values assigned during initialization before the constructor
	// body!
	height = -1;

        cache = null;

        initMatrixAndMetrics();
        initAdvCache();
    }

    private void writeObject(ObjectOutputStream out) throws IOException {

        cache = new int[256];
        for (int i=0; i < 256; i++) {
            cache[i] = -1;
        }
        serVersion = CURRENT_VERSION;

        out.defaultWriteObject();

        cache = null;
    }

    private float handleCharWidth(int ch) {
	return fontStrike.getCodePointAdvance(ch); // x-component of result only
    }

    // Uses advCache to get character width
    // It is incorrect to call this method for ch > 255
    private float getLatinCharWidth(char ch) {
        
        float w = advCache[ch];
        if (w == UNKNOWN_WIDTH) {
            w = handleCharWidth(ch);
            advCache[ch] = w;
        }
        return w;
    }

    public FontRenderContext getFRC() {

	return frc;
    }

    public int charWidth(char ch) {
        // default metrics for compatibility with legacy code
        float w;
        if (ch < 0x100) {
            w = getLatinCharWidth(ch);
        }
        else {
            w = handleCharWidth(ch);
        }
        return (int)(0.5 + w);
    }

    public int charWidth(int ch) {
        if (!Character.isValidCodePoint(ch)) {
	    ch = 0xffff;
	}

	float w = handleCharWidth(ch);

        return (int)(0.5 + w);
    }

    private boolean requiresLayout(char c) {
	return ((c >= 0x0590 && c < 0x0e80) ||
		(c >= 0x202a && c < 0x202f) ||
		(c >= 0xd800 && c < 0xe000));

	// hebrew, arabic, indic, thai, lao (and syriac, thaana)
	// or bidi control character (might not need unless there is hebrew or arabic, but here to be safe
	// or surrogates
    }

    public int stringWidth(String str) {

        int length = str.length();
        float width = 0;
        for (int i=0; i < length; i++) {
            char ch = str.charAt(i);
            if (ch < 0x100) {
                width += getLatinCharWidth(ch);
            }
            else if (requiresLayout(ch)) {
                width = new TextLayout(str, font, frc).getAdvance();
                break;
            }
            else {
                width += handleCharWidth(ch);
            }
        }

        return (int) (0.5 + width);
    }    
    
    public int charsWidth(char data[], int off, int len) {

        float width = 0;
        int limit = off + len;
        for (int i=off; i < limit; i++) {
            char ch = data[i];
            if (ch < 0x100) {
                width += getLatinCharWidth(ch);
            }
            else if (requiresLayout(ch)) {
                String str = new String(data, off, len);
                width = new TextLayout(str, font, frc).getAdvance();
                break;
            }
            else {
                width += handleCharWidth(ch);
            }
        }

        return (int) (0.5 + width);
    }    
    
    /**
     * Gets the advance widths of the first 256 characters in the 
     * <code>Font</code>.  The advance is the
     * distance from the leftmost point to the rightmost point on the
     * character's baseline.  Note that the advance of a
     * <code>String</code> is not necessarily the sum of the advances 
     * of its characters.
     * @return    an array storing the advance widths of the
     *                 characters in the <code>Font</code>
     *                 described by this <code>FontMetrics</code> object.
     */
    // More efficient than base class implementation - reuses existing cache
    public int[] getWidths() {
        int[] widths = new int[256];
	for (char ch = 0 ; ch < 256 ; ch++) {
            float w = advCache[ch];
            if (w == UNKNOWN_WIDTH) {
                w = advCache[ch] = handleCharWidth(ch);
            }
            widths[ch] = (int) (0.5 + w);
	}
	return widths;
    }

    public int getMaxAdvance() {
	return (int)(0.99f + this.maxAdvance);
    }

  /*
   * Returns the typographic ascent of the font. This is the maximum distance
   * glyphs in this font extend above the base line (measured in typographic
   * units).
   */
    public int getAscent() {
        return (int)(roundingUpValue + this.ascent);
    }

  /*
   * Returns the typographic descent of the font. This is the maximum distance
   * glyphs in this font extend below the base line.
   */
    public int getDescent() {
        return (int)(roundingUpValue + this.descent);
    }

    public int getLeading() {
	// nb this ensures the sum of the results of the public methods
	// for leading, ascent & descent sum to height.
	// if the calculations in any other methods change this needs
	// to be changed too.
	// the 0.95 value used here and in the other methods allows some
	// tiny fraction of leeway before rouding up. A higher value (0.99)
	// caused some excessive rounding up.
	return
	    (int)(roundingUpValue + descent + leading) -
	    (int)(roundingUpValue + descent);
    }

    // height is calculated as the sum of two separately rounded up values
    // because typically clients use ascent to determine the y location to
    // pass to drawString etc and we need to ensure that the height has enough
    // space below the baseline to fully contain any descender.
    public int getHeight() {

        if (height < 0) {
	    height = getAscent() + (int)(roundingUpValue + descent + leading);
        }
        return height;
    }
}
