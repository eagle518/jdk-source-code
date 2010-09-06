/* @(#)FontStrikeDesc.java	1.2 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.geom.AffineTransform;

/*
 * This class encapsulates every thing needed that distinguishes a strike.
 * It can be used as a key to locate a FontStrike in a Hashmap/cache.
 * It is not mutatable, but contains mutatable AffineTransform objects,
 * which for performance reasons it does not keep private copies of.
 * Therefore code constructing these must pass in transforms it guarantees
 * not to mutate.
 */
public class FontStrikeDesc {

    /* devTx is to get an inverse transform to get user space values
     * for metrics. Its not used otherwise, as the glyphTx is the important
     * one. But it does mean that a strike representing a 6pt font and identity
     * graphics transform is not equal to one for a 12 pt font and 2x scaled
     * graphics transform. Its likely to be very rare that this causes
     * duplication.
     */
    AffineTransform devTx;
    AffineTransform glyphTx; // all of ptSize, Font tx and Graphics tx.
    int style;
    boolean aaHint;
    boolean fmHint;
    private int hashCode;

    public int hashCode() {
	/* Can cache hashcode since a strike(desc) is immutable.*/
	if (hashCode == 0) {
	    hashCode = glyphTx.hashCode() + devTx.hashCode() +
		+ (aaHint ? 64 : 32) + (fmHint ? 16 : 8) + style;
	}
	return hashCode;
    }

    public boolean equals(Object obj) {
	try {
	    FontStrikeDesc desc = (FontStrikeDesc)obj;
	    return (desc.style  == this.style &&
		    desc.aaHint == this.aaHint &&
		    desc.fmHint == this.fmHint &&
		    desc.glyphTx.equals(this.glyphTx) &&
		    desc.devTx.equals(this.devTx));
	} catch (Exception e) {
	    /* class cast or NP exceptions should not happen often, if ever,
	     * and I am hoping that this is faster than an instanceof check.
	     */
	    return false;
	}
    }

    public FontStrikeDesc() {
	// used with init
    }

    public void init(AffineTransform devAT, AffineTransform at,
		     int style, boolean aa, boolean fm) {
	this.devTx = devAT;
	this.glyphTx = at; // not cloning. Callers trusted to not mutate "at"
	this.style = style;
	this.aaHint = aa;
	this.fmHint = fm;
	this.hashCode = 0; // must reset as init() is so can re-use a FSD.
    }

    public FontStrikeDesc(AffineTransform devAt, AffineTransform at,
			  int fStyle, boolean aa, boolean fm) {
	devTx = devAt;
	glyphTx = at; // not cloning glyphTx. Callers trusted to not mutate it.
	style = fStyle;
	aaHint = aa;
	fmHint = fm;
    }

    FontStrikeDesc(FontStrikeDesc desc) {
	devTx = desc.devTx;
	// Clone the TX in this case as this is called when its known
	// that "desc" is being re-used by its creator.
	glyphTx = (AffineTransform)desc.glyphTx.clone();
	style = desc.style;
	aaHint = desc.aaHint;
	fmHint = desc.fmHint;
	hashCode = desc.hashCode;
    }

    public String toString() {
	return "FontStrikeDesc: Style="+style+ " AA="+aaHint+ " FM="+fmHint+
	    " devTx="+devTx+ " devTx.FontTx.ptSize="+glyphTx;
    }
}
