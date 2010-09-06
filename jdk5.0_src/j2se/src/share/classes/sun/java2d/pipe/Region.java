/*
 * @(#)Region.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.Rectangle;

/**
 * This class encapsulates a definition of a two dimensional region which
 * consists of a number of Y ranges each containing multiple X bands.
 *
 * A rectangular Region is allowed to have a null band list in which
 * case the rectangular shape is defined by the bounding box parameters
 * (lox, loy, hix, hiy).
 *
 * The band list, if present, consists of a list of rows in ascending Y
 * order, ending at endIndex which is the index beyond the end of the
 * last row.  Each row consists of at least 3 + 2n entries (n >= 1)
 * where the first 3 entries specify the Y range as start, end, and
 * the number of X ranges in that Y range.  These 3 entries are
 * followed by pairs of X coordinates in ascending order:
 *
 * bands[rowstart+0] = Y0;        // starting Y coordinate
 * bands[rowstart+1] = Y1;        // ending Y coordinate - endY > startY
 * bands[rowstart+2] = N;         // number of X bands - N >= 1
 *
 * bands[rowstart+3] = X10;       // starting X coordinate of first band
 * bands[rowstart+4] = X11;       // ending X coordinate of first band
 * bands[rowstart+5] = X20;       // starting X coordinate of second band
 * bands[rowstart+6] = X21;       // ending X coordinate of second band
 * ...
 * bands[rowstart+3+N*2-2] = XN0; // starting X coord of last band
 * bands[rowstart+3+N*2-1] = XN1; // ending X coord of last band
 *
 * bands[rowstart+3+N*2] = ...    // start of next Y row
 */
public class Region {
    public static final int INIT_SIZE = 50;
    public static final int GROW_SIZE = 50;

    int lox;
    int loy;
    int hix;
    int hiy;

    int endIndex;
    int[] bands;

    private static native void initIDs();

    static {
	initIDs();
    }

    /*
     * Adds the dimension <code>dim</code> to the coordinate
     * <code>start</code> with appropriate clipping.  If
     * <code>dim</code> is non-positive then the method returns
     * the start coordinate.  If the sum overflows an integer
     * data type then the method returns <code>Integer.MAX_VALUE</code>.
     */
    public static int dimAdd(int start, int dim) {
	if (dim <= 0) return start;
	if ((dim += start) < start) return Integer.MAX_VALUE;
	return dim;
    }

    private Region(int lox, int loy, int hix, int hiy) {
	this.lox = lox;
	this.loy = loy;
	this.hix = hix;
	this.hiy = hiy;
    }

    /*
     * Returns a Region object with a rectangle of interest specified
     * by the indicated Rectangle object.
     *
     * This method can also be used to create a simple rectangular
     * region.
     */
    public static Region getInstance(Rectangle r) {
	return Region.getInstanceXYWH(r.x, r.y, r.width, r.height);
    }

    /*
     * Returns a Region object with a rectangle of interest specified
     * by the indicated rectangular area in x, y, width, height format.
     *
     * This method can also be used to create a simple rectangular
     * region.
     */
    public static Region getInstanceXYWH(int x, int y, int w, int h) {
	return Region.getInstanceXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    /*
     * Returns a Region object with a rectangle of interest specified
     * by the indicated span array.
     *
     * This method can also be used to create a simple rectangular
     * region.
     */
    public static Region getInstance(int box[]) {
	return new Region(box[0], box[1], box[2], box[3]);
    }

    /*
     * Returns a Region object with a rectangle of interest specified
     * by the indicated rectangular area in lox, loy, hix, hiy format.
     *
     * This method can also be used to create a simple rectangular
     * region.
     */
    public static Region getInstanceXYXY(int lox, int loy, int hix, int hiy) {
	return new Region(lox, loy, hix, hiy);
    }

    /*
     * Sets the rectangle of interest for storing and returning
     * region bands.
     *
     * This method can also be used to initialize a simple rectangular
     * region.
     */
    public void setOutputArea(Rectangle r) {
	setOutputAreaXYWH(r.x, r.y, r.width, r.height);
    }

    /*
     * Sets the rectangle of interest for storing and returning
     * region bands.  The rectangle is specified in x, y, width, height
     * format and appropriate clipping is performed as per the method
     * <code>dimAdd</code>.
     *
     * This method can also be used to initialize a simple rectangular
     * region.
     */
    public void setOutputAreaXYWH(int x, int y, int w, int h) {
	setOutputAreaXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    /*
     * Sets the rectangle of interest for storing and returning
     * region bands.  The rectangle is specified as a span array.
     *
     * This method can also be used to initialize a simple rectangular
     * region.
     */
    public void setOutputArea(int box[]) {
	this.lox = box[0];
	this.loy = box[1];
	this.hix = box[2];
	this.hiy = box[3];
    }

    /*
     * Sets the rectangle of interest for storing and returning
     * region bands.  The rectangle is specified in lox, loy,
     * hix, hiy format.
     *
     * This method can also be used to initialize a simple rectangular
     * region.
     */
    public void setOutputAreaXYXY(int lox, int loy, int hix, int hiy) {
	this.lox = lox;
	this.loy = loy;
	this.hix = hix;
	this.hiy = hiy;
    }

    /*
     * Appends the list of spans returned from the indicated
     * SpanIterator.  Each span must be at a higher starting
     * Y coordinate than the previous data or it must have a
     * Y range equal to the highest Y band in the region and a
     * higher X coordinate than any of the spans in that band.
     */
    public void appendSpans(SpanIterator si) {
	int[] box = new int[6];

	while (si.nextSpan(box)) {
	    appendSpan(box);
	}

	endRow(box);
	calcBBox();
    }

    /*
     * Returns a Region object that represents the intersection of
     * this object with the specified Rectangle.  The return value
     * may be this same object if no clipping occurs.
     */
    public Region getIntersection(Rectangle r) {
	return getIntersectionXYWH(r.x, r.y, r.width, r.height);
    }

    /*
     * Returns a Region object that represents the intersection of
     * this object with the specified rectangular area.  The return
     * value may be this same object if no clipping occurs.
     */
    public Region getIntersectionXYWH(int x, int y, int w, int h) {
	return getIntersectionXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    /*
     * Returns a Region object that represents the intersection of
     * this object with the specified rectangular area.  The return
     * value may be this same object if no clipping occurs.
     */
    public Region getIntersectionXYXY(int lox, int loy, int hix, int hiy) {
	if (isInsideXYXY(lox, loy, hix, hiy)) {
	    return this;
	}
	Region ret = new Region((lox < this.lox) ? this.lox : lox,
				(loy < this.loy) ? this.loy : loy,
				(hix > this.hix) ? this.hix : hix,
				(hiy > this.hiy) ? this.hiy : hiy);
	if (bands != null) {
	    ret.appendSpans(this.getSpanIterator());
	}
	return ret;
    }

    /*
     * Returns a Region object that represents the bounds of the
     * intersection of this object with the bounds of the specified
     * Region object.
     *
     * The return value may be this same object or the argument
     * Region object if no clipping occurs.
     */
    public Region getIntersection(Region r) {
	if (this.isInsideQuickCheck(r)) {
	    return this;
	}
	if (r.isInsideQuickCheck(this)) {
	    return r;
	}
	Region ret = new Region((r.lox < this.lox) ? this.lox : r.lox,
				(r.loy < this.loy) ? this.loy : r.loy,
				(r.hix > this.hix) ? this.hix : r.hix,
				(r.hiy > this.hiy) ? this.hiy : r.hiy);
	SpanIterator si = null;
	if (this.bands != null) {
	    si = this.getSpanIterator();
	}
	if (r.bands != null) {
	    si = (si == null) ? r.getSpanIterator() : r.filter(si);
	}
	if (si != null) {
	    // REMIND: this may not work because filtered SpanIterators
	    // are not guaranteed to return ordered spans...
	    ret.appendSpans(si);
	}
	return ret;
    }

    /*
     * Returns a Region object that represents the bounds of the
     * intersection of this object with the bounds of the specified
     * Region object.
     *
     * The return value may be this same object if no clipping occurs
     * and this Region is rectangular.
     */
    public Region getBoundsIntersection(Rectangle r) {
	return getBoundsIntersectionXYWH(r.x, r.y, r.width, r.height);
    }

    /*
     * Returns a Region object that represents the bounds of the
     * intersection of this object with the bounds of the specified
     * rectangular area in x, y, width, height format.
     *
     * The return value may be this same object if no clipping occurs
     * and this Region is rectangular.
     */
    public Region getBoundsIntersectionXYWH(int x, int y, int w, int h) {
	return getBoundsIntersectionXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    public Region getBoundsIntersectionXYXY(int lox, int loy,
					    int hix, int hiy)
    {
	if (this.bands == null &&
	    this.lox >= lox && this.loy >= loy &&
	    this.hix <= hix && this.hiy <= hiy)
	{
	    return this;
	}
	return new Region((lox < this.lox) ? this.lox : lox,
			  (loy < this.loy) ? this.loy : loy,
			  (hix > this.hix) ? this.hix : hix,
			  (hiy > this.hiy) ? this.hiy : hiy);
    }

    /*
     * Returns a Region object that represents the intersection of
     * this object with the bounds of the specified Region object.
     * The return value may be this same object or the argument
     * Region object if no clipping occurs and the Regions are
     * rectangular.
     */
    public Region getBoundsIntersection(Region r) {
	if (this.encompasses(r)) {
	    return r;
	}
	if (r.encompasses(this)) {
	    return this;
	}
	return new Region((r.lox < this.lox) ? this.lox : r.lox,
			  (r.loy < this.loy) ? this.loy : r.loy,
			  (r.hix > this.hix) ? this.hix : r.hix,
			  (r.hiy > this.hiy) ? this.hiy : r.hiy);
    }

    /*
     * Appends a single span defined by the 4 parameters
     * spanlox, spanloy, spanhix, spanhiy.
     * This span must be at a higher starting Y coordinate than
     * the previous data or it must have a Y range equal to the
     * highest Y band in the region and a higher X coordinate
     * than any of the spans in that band.
     */
    private void appendSpan(int box[]) {
	int spanlox, spanloy, spanhix, spanhiy;
	if ((spanlox = box[0]) < lox) spanlox = lox;
	if ((spanloy = box[1]) < loy) spanloy = loy;
	if ((spanhix = box[2]) > hix) spanhix = hix;
	if ((spanhiy = box[3]) > hiy) spanhiy = hiy;
	if (spanhix <= spanlox || spanhiy <= spanloy) {
	    return;
	}

	int curYrow = box[4];
	if (endIndex == 0 || spanloy >= bands[curYrow + 1]) {
	    if (bands == null) {
		bands = new int[INIT_SIZE];
	    } else {
		needSpace(5);
		endRow(box);
		curYrow = box[4];
	    }
	    bands[endIndex++] = spanloy;
	    bands[endIndex++] = spanhiy;
	    bands[endIndex++] = 0;
	} else if (spanloy == bands[curYrow] &&
		   spanhiy == bands[curYrow + 1] &&
		   spanlox >= bands[endIndex - 1]) {
	    if (spanlox == bands[endIndex - 1]) {
		bands[endIndex - 1] = spanhix;
		return;
	    }
	    needSpace(2);
	} else {
	    throw new InternalError("bad span");
	}
	bands[endIndex++] = spanlox;
	bands[endIndex++] = spanhix;
	bands[curYrow + 2]++;
    }

    private void needSpace(int num) {
	if (endIndex + num >= bands.length) {
	    int[] newbands = new int[bands.length + GROW_SIZE];
	    System.arraycopy(bands, 0, newbands, 0, endIndex);
	    bands = newbands;
	}
    }

    private void endRow(int box[]) {
	int cur = box[4];
	int prev = box[5];
	if (cur > prev) {
	    int[] bands = this.bands;
	    if (bands[prev + 1] == bands[cur] &&
		bands[prev + 2] == bands[cur + 2])
	    {
		int num = bands[cur + 2] * 2; 
		cur += 3;
		prev += 3;
		while (num > 0) {
		    if (bands[cur++] != bands[prev++]) {
			break;
		    }
		    num--;
		}
		if (num == 0) {
		    // prev == box[4]
		    bands[box[5] + 1] = bands[prev + 1];
		    endIndex = prev;
		    return;
		}
	    }
	}
	box[5] = box[4];
	box[4] = endIndex;
    }

    private void calcBBox() {
	int[] bands = this.bands;
	if (endIndex <= 5) {
	    if (endIndex == 0) {
		lox = loy = hix = hiy = 0;
	    } else {
		loy = bands[0];
		hiy = bands[1];
		lox = bands[3];
		hix = bands[4];
		endIndex = 0;
	    }
	    this.bands = null;
	    return;
	}
	int lox = this.hix;
	int hix = this.lox;
	int hiyindex = 0;

	int i = 0;
	while (i < endIndex) {
	    hiyindex = i;
	    int numbands = bands[i + 2];
	    i += 3;
	    if (lox > bands[i]) {
		lox = bands[i];
	    }
	    i += numbands * 2;
	    if (hix < bands[i - 1]) {
		hix = bands[i - 1];
	    }
	}

	this.lox = lox;
	this.loy = bands[0];
	this.hix = hix;
	this.hiy = bands[hiyindex + 1];
    }

    /*
     * Returns the lowest X coordinate in the Region.
     */
    public final int getLoX() {
	return lox;
    }

    /*
     * Returns the lowest Y coordinate in the Region.
     */
    public final int getLoY() {
	return loy;
    }

    /*
     * Returns the highest X coordinate in the Region.
     */
    public final int getHiX() {
	return hix;
    }

    /*
     * Returns the highest Y coordinate in the Region.
     */
    public final int getHiY() {
	return hiy;
    }

    /*
     * Returns the width of this Region clipped to the range (0 - MAX_INT).
     */
    public final int getWidth() {
	if (hix < lox) return 0;
	int w;
	if ((w = hix - lox) < 0) {
	    w = Integer.MAX_VALUE;
	}
	return w;
    }

    /*
     * Returns the height of this Region clipped to the range (0 - MAX_INT).
     */
    public final int getHeight() {
	if (hiy < loy) return 0;
	int h;
	if ((h = hiy - loy) < 0) {
	    h = Integer.MAX_VALUE;
	}
	return h;
    }

    /*
     * Returns true iff this Region encloses no area.
     */
    public boolean isEmpty() {
	return (hix <= lox || hiy <= loy);
    }

    /*
     * Returns true iff this Region represents a single simple
     * rectangular area.
     */
    public boolean isRectangular() {
	return (bands == null);
    }

    /*
     * Returns true iff this Region lies inside the indicated
     * rectangular area specified in x, y, width, height format
     * with appropriate clipping performed as per the dimAdd method.
     */
    public boolean isInsideXYWH(int x, int y, int w, int h) {
	return isInsideXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    /*
     * Returns true iff this Region lies inside the indicated
     * rectangular area specified in lox, loy, hix, hiy format.
     */
    public boolean isInsideXYXY(int lox, int loy, int hix, int hiy) {
	return (this.lox >= lox && this.loy >= loy &&
		this.hix <= hix && this.hiy <= hiy);
		
    }

    /*
     * Quickly checks if this Region lies inside the specified
     * Region object.
     *
     * This method will return false if the specified Region
     * object is not a simple rectangle.
     */
    public boolean isInsideQuickCheck(Region r) {
	return (r.bands == null &&
		r.lox <= this.lox && r.loy <= this.loy &&
		r.hix >= this.hix && r.hiy >= this.hiy);
    }

    /*
     * Quickly checks if this Region intersects the specified
     * rectangular area specified in lox, loy, hix, hiy format.
     *
     * This method tests only against the bounds of this region
     * and does not bother to test if the rectangular region
     * actually intersects any bands.
     */
    public boolean intersectsQuickCheckXYXY(int lox, int loy,
					    int hix, int hiy)
    {
	return (hix > this.lox && lox < this.hix &&
		hiy > this.loy && loy < this.hiy);
    }

    /*
     * Quickly checks if this Region surrounds the specified
     * Region object.
     *
     * This method will return false if this Region object is
     * not a simple rectangle.
     */
    public boolean encompasses(Region r) {
	return (this.bands == null &&
		this.lox <= r.lox && this.loy <= r.loy &&
		this.hix >= r.hix && this.hiy >= r.hiy);
    }

    /*
     * Quickly checks if this Region surrounds the specified
     * rectangular area specified in x, y, width, height format.
     *
     * This method will return false if this Region object is
     * not a simple rectangle.
     */
    public boolean encompassesXYWH(int x, int y, int w, int h) {
	return encompassesXYXY(x, y, dimAdd(x, w), dimAdd(y, h));
    }

    /*
     * Quickly checks if this Region surrounds the specified
     * rectangular area specified in lox, loy, hix, hiy format.
     *
     * This method will return false if this Region object is
     * not a simple rectangle.
     */
    public boolean encompassesXYXY(int lox, int loy, int hix, int hiy) {
	return (this.bands == null &&
		this.lox <= lox && this.loy <= loy &&
		this.hix >= hix && this.hiy >= hiy);
    }

    /*
     * Gets the bbox of the available spans, clipped to the OutputArea.
     */
    public void getBounds(int pathbox[]) {
	pathbox[0] = lox;
	pathbox[1] = loy;
	pathbox[2] = hix;
	pathbox[3] = hiy;
    }

    /*
     * Clips the indicated bbox array to the bounds of this Region.
     */
    public void clipBoxToBounds(int bbox[]) {
	if (bbox[0] < lox) bbox[0] = lox;
	if (bbox[1] < loy) bbox[1] = loy;
	if (bbox[2] > hix) bbox[2] = hix;
	if (bbox[3] > hiy) bbox[3] = hiy;
    }

    /*
     * Gets an iterator object to iterate over the spans in this region.
     */
    public RegionIterator getIterator() {
	return new RegionIterator(this);
    }

    /*
     * Gets a span iterator object that iterates over the spans in this region
     */
    public SpanIterator getSpanIterator() {
	return new RegionSpanIterator(this);
    }
    
    /*
     * Gets a span iterator object that iterates over the spans in this region
     * but clipped to the bounds given in the argument (xlo, ylo, xhi, yhi).
     */
    public SpanIterator getSpanIterator(int bbox[]) {
	SpanIterator result = getSpanIterator();
	result.intersectClipBox(bbox[0], bbox[1], bbox[2], bbox[3]);
	return result;
    }

    /*
     * Returns a SpanIterator that is the argument iterator filtered by
     * this region.
     */
    public SpanIterator filter(SpanIterator si) {
	if (bands == null) {
	    si.intersectClipBox(lox, loy, hix, hiy);
	} else {
	    si = new RegionClipSpanIterator(this, si);
	}
	return si;
    }
}
