/*
 * @(#)FontInfo.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.loops;

import java.awt.Font;
import java.awt.geom.AffineTransform;

import sun.font.Font2D;
import sun.font.FontStrike;
import sun.font.FontStrikeDesc;

/*
 * A FontInfo object holds all calculated or derived data needed
 * to handle rendering operations based on a particular set of
 * Graphics2D rendering attributes.
 * Note that this does not use a Font2DHandle, and also has a reference
 * to the strike which also references the Font2D.
 * So presently, until SG2D objects no longer reference this FontInfo,
 * there is still some potential for a bad Font2D to be used for a short
 * time. I am reluctant to add the overhead of that machinery here without
 * a proven benefit.
 */
public class FontInfo implements Cloneable {
    public Font font;
    public Font2D font2D;
    public FontStrike fontStrike;
    public double[] devTx;
    public double[] glyphTx;
    public int pixelHeight;
    public float originX;
    public float originY;

    public String mtx(double[] matrix) {
    	return ("["+
		matrix[0]+", "+
		matrix[1]+", "+
		matrix[2]+", "+
		matrix[3]+
		"]");
    }

    public Object clone() {
	try {
	    return super.clone();
	} catch (CloneNotSupportedException e) {
	    return null;
	}
    }

    public String toString() {
	return ("FontInfo["+
		"font="+font+", "+
		"devTx="+mtx(devTx)+", "+
		"glyphTx="+mtx(glyphTx)+", "+
		"pixelHeight="+pixelHeight+", "+
		"origin=("+originX+","+originY+"), "+
		"]");
    }
}
