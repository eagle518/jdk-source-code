/*
 * @(#)GlyphList.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.Font;
import java.awt.font.GlyphVector;
import java.awt.font.FontRenderContext;
import sun.java2d.loops.FontInfo;

/*
 * This class represents a list of actual renderable glyphs.
 * It can be constructed from a number of text sources, representing
 * the various ways in which a programmer can ask a Graphics2D object
 * to render some text.  Once constructed, it provides a way of iterating
 * through the device metrics and graybits of the individual glyphs that
 * need to be rendered to the screen.
 *
 * Note that this class holds pointers to native data which must be
 * disposed.  It is not marked as finalizable since it is intended
 * to be very lightweight and finalization is a comparitively expensive
 * procedure.  The caller must specifically use try{} finally{} to
 * manually ensure that the object is disposed after use, otherwise
 * native data structures might be leaked.
 *
 * Here is a code sample for using this class:
 *
 * public void drawString(String str, FontInfo info, float x, float y) {
 *     GlyphList gl = GlyphList.getInstance();
 *     try {
 *         gl.setFromString(info, str, x, y);
 *         int strbounds[] = gl.getBounds();
 *         int numglyphs = gl.getNumGlyphs();
 *         for (int i = 0; i < numglyphs; i++) {
 *             gl.setGlyphIndex(i);
 *             int metrics[] = gl.getMetrics();
 *             byte bits[] = gl.getGrayBits();
 *             int glyphx = metrics[0];
 *             int glyphy = metrics[1];
 *             int glyphw = metrics[2];
 *             int glyphh = metrics[3];
 *             int off = 0;
 *             for (int j = 0; j < glyphh; j++) {
 *                 for (int i = 0; i < glyphw; i++) {
 *                     int dx = glyphx + i;
 *                     int dy = glyphy + j;
 *                     int alpha = bits[off++];
 *                     drawPixel(alpha, dx, dy);
 *                 }
 *             }
 *         }
 *     } finally {
 *         gl.dispose();
 *     }
 * }
 */
public final class GlyphList {
    private static final int MINGRAYLENGTH = 1024;
    private static final int MAXGRAYLENGTH = 8192;
    private static final int DEFAULT_LENGTH = 32;

    int glyphindex;
    int metrics[];
    byte graybits[];
    Object strikelist; // hold multiple strikes during rendering of complex gv

    /* In normal usage, the same GlyphList will get recycled, so
     * it makes sense to allocate arrays that will get reused along with
     * it, rather than generating garbage. Garbage will be generated only
     * in MP envts where multiple threads are executing. Throughput should
     * still be higher in those cases.
     */
    int len = 0;
    int maxLen = 0;
    int maxPosLen = 0;
    int glyphData[];
    char chData[];
    long images[];
    float positions[];
    float x, y;
    float gposx, gposy;
    boolean usePositions;

    /* This scheme creates a singleton GlyphList which is checked out
     * for use. Callers who find its checked out create one that after use
     * is discarded. This means that in a MT-rendering environment,
     * there's no need to synchronise except for that one instance.
     * Fewer threads will then need to synchronise, perhaps helping
     * throughput on a MP system. If for some reason the reusable
     * GlyphList is checked out for a long time (or never returned?) then
     * we would end up always creating new ones. That situation should not
     * occur and if if did, it would just lead to some extra garbage being
     * created.
     */
    private static GlyphList reusableGL = new GlyphList();
    private static boolean inUse;


    void ensureCapacity(int len) {
      /* Note len must not be -ve! only setFromChars should be capable
       * of passing down a -ve len, and this guards against it.
       */
        if (len < 0) { 
	  len = 0;
        }
        if (usePositions && len > maxPosLen) {
            positions = new float[len * 2 + 2];
	    maxPosLen = len;
        }

        if (maxLen == 0 || len > maxLen) {
            glyphData = new int[len];
            chData = new char[len];
            images = new long[len];
            maxLen = len;
        }
    }

    private GlyphList() {
//         ensureCapacity(DEFAULT_LENGTH);
    }

//     private GlyphList(int arraylen) {
//          ensureCapacity(arraylen);
//     }

    public static GlyphList getInstance() {
	/* The following heuristic is that if the reusable instance is
	 * in use, it probably still will be in a micro-second, so avoid
	 * synchronising on the class and just allocate a new instance.
	 * The cost is one extra boolean test for the normal case, and some
	 * small number of cases where we allocate an extra object when
	 * in fact the reusable one would be freed very soon.
	 */
	if (inUse) {
	    return new GlyphList();
	} else {
	    synchronized(GlyphList.class) {
		if (inUse) {
		    return new GlyphList();
		} else {
		    inUse = true;
		    return reusableGL;
		}
	    }
	}
    }

    /* In some cases the caller may be able to estimate the size of
     * array needed, and it will usually be long enough. This avoids
     * the unnecessary reallocation that occurs if our default
     * values are too small. This is useful because this object
     * will be discarded so the re-allocation overhead is high.
     */
//     public static GlyphList getInstance(int sz) {
// 	if (inUse) {
// 	    return new GlyphList(sz);
// 	} else {
// 	    synchronized(GlyphList.class) {
// 		if (inUse) {
// 		    return new GlyphList();
// 		} else {
// 		    inUse = true;
// 		    return reusableGL;
// 		}
// 	    }
// 	}
//     }

    /* GlyphList is in an invalid state until setFrom* method is called.
     * After obtaining a new GlyphList it is the caller's responsibility
     * that one of these methods is executed before handing off the
     * GlyphList
     */

    public boolean setFromString(FontInfo info, String str, float x, float y) {
	this.x = x;
	this.y = y;
	len = str.length();
        ensureCapacity(len);
	str.getChars(0, len, chData, 0);
	return mapChars(info, len);
    }

    public boolean setFromChars(FontInfo info, char[] chars, int off, int alen,
				float x, float y) {
	this.x = x;
	this.y = y;
	len = alen;
	if (alen < 0) {
	    len = 0;
	} else {
	    len = alen;
	}
        ensureCapacity(len);
	System.arraycopy(chars, off, chData, 0, len);
	return mapChars(info, len);
    }

    private final boolean mapChars(FontInfo info, int len) {
	/* REMIND.Is it worthwhile for the iteration to convert 
	 * chars to glyph ids to directly map to images?  
	 */
 	if (info.font2D.getMapper().charsToGlyphsNS(len, chData, glyphData)) {
	    return false;
	}
	info.fontStrike.getGlyphImagePtrs(glyphData, images, len);
	glyphindex = -1;
	return true;
    }


    public void setFromGlyphVector(FontInfo info, GlyphVector gv,
				   float x, float y) {
        this.x = x;
        this.y = y;
        StandardGlyphVector sgv = StandardGlyphVector.getStandardGV(gv);
	// call before ensureCapacity :-
        usePositions = sgv.needsPositions(info.devTx);
        len = sgv.getNumGlyphs();
        ensureCapacity(len);
        strikelist = sgv.setupGlyphImages(images,
					  usePositions ? positions : null,
					  info.devTx);

        glyphindex = -1;
    }

    public int[] getBounds() {
	/* We co-opt the 4 element array that holds per glyph metrics in order
	 * to return the bounds. So a caller must copy the data out of the
	 * array before calling any other methods on this GlyphList
	 */
	if (glyphindex >= 0) {
	    throw new InternalError("calling getBounds after setGlyphIndex");
	}
	if (metrics == null) {
	    metrics = new int[4];
	}
	/* gposx and gposy are used to accumulate the advance */
	gposx = x;
	gposy = y;
	fillBounds(metrics);
	return metrics;
    }

    /* This method now assumes "state", so must be called 0->len
     * The metrics it returns are accumulated on the fly
     * So it could be renamed "nextGlyph()".
     * Note that a laid out GlyphVector which has assigned glyph positions
     * doesn't have this stricture..
     */
    public void setGlyphIndex(int i) {
	glyphindex = i;
	float gx =
	    StrikeCache.unsafe.getFloat(images[i]+StrikeCache.topLeftXOffset);
	float gy =
	    StrikeCache.unsafe.getFloat(images[i]+StrikeCache.topLeftYOffset);

      	if (usePositions) {
	    metrics[0] = (int)(positions[(i<<1)]   + gposx + gx);
	    metrics[1] = (int)(positions[(i<<1)+1] + gposy + gy);
	} else {
	    metrics[0] = (int)(gposx + gx);
	    metrics[1] = (int)(gposy + gy);
	    /* gposx and gposy are used to accumulate the advance */
	    gposx += StrikeCache.unsafe.getFloat
		(images[i]+StrikeCache.xAdvanceOffset);
	    gposy += StrikeCache.unsafe.getFloat
		(images[i]+StrikeCache.yAdvanceOffset);
	}
	metrics[2] =
	    StrikeCache.unsafe.getChar(images[i]+StrikeCache.widthOffset);
	metrics[3] =
	    StrikeCache.unsafe.getChar(images[i]+StrikeCache.heightOffset);
    }

    public int[] getMetrics() {
	return metrics;
    }

    public byte[] getGrayBits() {
	int len = metrics[2] * metrics[3];
	if (graybits == null) {
	    graybits = new byte[Math.max(len, MINGRAYLENGTH)];
	} else {
	    if (len > graybits.length) {
		graybits = new byte[len];
	    }
	}
	long pixelDataAddress;
	if (StrikeCache.nativeAddressSize == 4) {
	    pixelDataAddress = 0xffffffff &
		StrikeCache.unsafe.getInt(images[glyphindex] +
					  StrikeCache.pixelDataOffset);
	} else {
	    pixelDataAddress =
	    StrikeCache.unsafe.getLong(images[glyphindex] +
				       StrikeCache.pixelDataOffset);
	}
	if (pixelDataAddress == 0L) {
	    return graybits;
	}
	/* unsafe is supposed to be fast, but I doubt if this loop can beat
	 * a native call which does a getPrimitiveArrayCritical and a
	 * memcpy for the typical amount of image data (30-150 bytes)
	 * Consider a native method if there is a performance problem (which
	 * I haven't seen so far).
	 */
	for (int i=0; i<len; i++) {
	    graybits[i] = StrikeCache.unsafe.getByte(pixelDataAddress+i);
	}
	return graybits;
    }

    /* There's a reference equality test overhead here, but it allows us
     * to avoid synchronizing for GL's that will just be GC'd. This
     * helps MP throughput.
     */
    public void dispose() {
	if (this == reusableGL) {
	    if (graybits != null && graybits.length > MAXGRAYLENGTH) {
		graybits = null;
	    }
	    usePositions = false;
	    strikelist = null; // deref if gvset it
	    inUse = false;
	}
    }

    /* The value here is for use by the rendering engine as it reflects
     * the number of glyphs in the array to be blitted. Surrogates pairs
     * may have two slots (the second of these being a dummy entry of the
     * invisible glyph), whereas an application client would expect only
     * one glyph. In other words don't propagate this value up to client code.
     *
     * {dlf} an application client should have _no_ expectations about the
     * number of glyphs per char.  This ultimately depends on the font
     * technology and layout process used, which in general clients will
     * know nothing about.
     */
    public int getNumGlyphs() {
	return len;
    }

    /* We re-do all this work as we iterate through the glyphs
     * but it seems unavoidable without re-working the Java TextRenderers.
     */
    private void fillBounds(int[] bounds) {
	/* Faster to access local variables in the for loop? */
	int xOffset = StrikeCache.topLeftXOffset;
	int yOffset = StrikeCache.topLeftYOffset;
	int wOffset = StrikeCache.widthOffset;
	int hOffset = StrikeCache.heightOffset;
	int xAdvOffset = StrikeCache.xAdvanceOffset;
	int yAdvOffset = StrikeCache.yAdvanceOffset;

	if (len == 0) {
	    bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
	    return;
	}
	float bx0, by0, bx1, by1;
	bx0 = by0 = Float.POSITIVE_INFINITY;
	bx1 = by1 = Float.NEGATIVE_INFINITY;

	int posIndex = 0;
	float glx = x;
	float gly = y;
	char gw, gh;
	float gx, gy, gx0, gy0, gx1, gy1;
	for (int i=0; i<len; i++) {
	    gx = StrikeCache.unsafe.getFloat(images[i]+xOffset);
	    gy = StrikeCache.unsafe.getFloat(images[i]+yOffset);
	    gw = StrikeCache.unsafe.getChar(images[i]+wOffset);
	    gh = StrikeCache.unsafe.getChar(images[i]+hOffset);

	    if (usePositions) {
		gx0 = positions[posIndex++] + gx;
		gy0 = positions[posIndex++] + gy;
	    } else {
		gx0 = glx + gx;
		gy0 = gly + gy;
		glx += StrikeCache.unsafe.getFloat(images[i]+xAdvOffset);
		gly += StrikeCache.unsafe.getFloat(images[i]+yAdvOffset);
	    }
	    gx1 = gx0 + gw;
	    gy1 = gy0 + gh;
	    if (bx0 > gx0) bx0 = gx0;
	    if (by0 > gy0) by0 = gy0;
	    if (bx1 < gx1) bx1 = gx1;
	    if (by1 < gy1) by1 = gy1;	
	}
	bounds[0] = (int)bx0;
	bounds[1] = (int)by0;
	bounds[2] = (int)bx1;
	bounds[3] = (int)by1;
    }
}
