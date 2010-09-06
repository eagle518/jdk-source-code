/* @(#)FileFontStrike.java	1.10 03/21/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.lang.ref.SoftReference;
import java.awt.Font;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.geom.GeneralPath;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.HashMap;


public class FileFontStrike extends PhysicalStrike {

    /* fffe and ffff are values we specially interpret as meaning
     * invisible glyphs.
     */
    static final int INVISIBLE_GLYPHS = 0x0fffe;

    private FileFont fileFont;

    /* The "metrics" information requested by clients is usually nothing
     * more than the horizontal advance of the character. So cache this
     * information.
     * This advance cache will take an extra (4 * numglyphs) bytes per
     * strike and profiling will be needed to distinguish if its beneficial.
     * The big advantage is the glyph Image pointers can then be more simply
     * distinguished as valid images.
     * Metrics other than horizontal advance is not cached in Java code but
     * if a valid glyph Image exists it will contain all the metrics
     * info (except for outline bounds which is even more rarely needed).
     */
    protected float[] horizontalAdvances;

    /* Outline bounds are used when printing and when drawing outlines
     * to the screen. On balance the relative rarity of these cases
     * and the fact that getting this requires generating a path at
     * the scaler level means that its probably OK to store these
     * in a Java-level hashmap as the trade-off between time and space.
     * Later can revisit whether to cache these at all, or elsewhere.
     * Should also profile whether subsequent to getting the bounds, the
     * outline itself is also requested. The 1.4 implementation doesn't
     * cache outlines so you could generate the path twice - once to get
     * the bounds and again to return the outline to the client.
     * If the two uses are coincident then also look into caching outlines.
     * One simple optimisation is that we could store the last single
     * outline retrieved. This assumes that bounds then outline will always
     * be retrieved for a glyph rather than retrieving bounds for all glyphs
     * then outlines for all glyphs.
     */
    HashMap boundsMap;
    SoftReference glyphMetricsMapRef;

    AffineTransform invertDevTx;

    boolean useNatives;
    NativeStrike[] nativeStrikes;

    FileFontStrike(FileFont fileFont, FontStrikeDesc desc) {
	super(fileFont, desc);
	this.fileFont = fileFont;

	if (desc.style != fileFont.style) {
	  /* If using algorithmic styling, the base values are
	   * boldness = 1.0, italic = 0.0. The superclass constructor
	   * initialises these.
	   */
	    if ((desc.style & Font.ITALIC) == Font.ITALIC &&
		(fileFont.style & Font.ITALIC) == 0) {
		algoStyle = true;
		italic = 0.7f;
	    }
	    if ((desc.style & Font.BOLD) == Font.BOLD &&
		((fileFont.style & Font.BOLD) == 0)) {
		algoStyle = true;
		boldness = 1.33f;
	    }
	}
	double[] matrix = new double[4];
	AffineTransform at = desc.glyphTx;
	at.getMatrix(matrix);
	if (!desc.devTx.isIdentity() &&
	    desc.devTx.getType() != AffineTransform.TYPE_TRANSLATION) {
	    try {
		invertDevTx = desc.devTx.createInverse();
	    } catch (NoninvertibleTransformException e) {
	    }
	}
	pScalerContext =
	    createScalerContext(fileFont.getScaler(), matrix,
				fileFont instanceof TrueTypeFont,
				desc.aaHint, desc.fmHint,
				algoStyle, boldness, italic);

	mapper = fileFont.getMapper();
	int numGlyphs = mapper.getNumGlyphs();

	/* Bad font. The scaler may already be the "null scaler"
	 * or less likely this context couldn't be created.
	 * In either case we create a minimal strike here that returns
	 * empty glyphs, empty metrics and de-register this font for
	 * future use. This isn't going to be completely seamless,
	 * the user may see artifacts, but we won't crash and only
	 * this font will be affected, and only for so long as this
	 * strike is in use.
	 */
	if (pScalerContext == 0L) {
	    if (FontManager.longAddresses) {
		longGlyphImages = new long[numGlyphs];
	    } else {
		intGlyphImages = new int[numGlyphs];
	    }	    
	    pScalerContext = getNullScalerContext(fileFont.getNullScaler());
	    FontManager.deRegisterBadFont(fileFont);
	    return;
	}

        if (fileFont.useNatives && !desc.aaHint && !algoStyle) {
            /* Check its a simple scale of a pt size in the range
	     * where native bitmaps typically exist (6-36 pts) */
            if (matrix[1] == 0.0 && matrix[2] == 0.0 &&
                matrix[0] >= 6.0 && matrix[0] <= 36.0 &&
		matrix[0] == matrix[3]) {
		useNatives = true;
                int numNatives = fileFont.nativeFonts.length;
                nativeStrikes = new NativeStrike[numNatives];
                /* Maybe initialise these strikes lazily?. But we
                 * know we need at least one
                 */
                for (int i=0; i<numNatives; i++) {
                    nativeStrikes[i] =
			new NativeStrike(fileFont.nativeFonts[i], desc, false);
                }
            }
        }

	this.disposer = new FontStrikeDisposer(fileFont, desc, pScalerContext);

	/* Always get the image and the advance together for natives
	 * as it eliminates the need to have the metrics only code
	 * also have a fallback to native. We only use natives for
	 * strikes with pt size <=36 so we are insulated from using
	 * excessive memory.
	 */
	getImageWithAdvance = (useNatives ||
	     (at.getScaleY() < 20.0 && (at.getType() & complexTX) == 0));

	/* Choosing here not caching advances for strikes that
	 * seem "atypical" cases, where the strike is likely transient,
	 * or not performance sensitive. Very large pixel sizes and
	 * transformations that involve a rotation component are examples.
	 * Also don't cache advances for large fonts. This heuristic is also
	 * used to choose whether its appropriate to automatically retrieve
	 * images along with metrics. The theory is that metrics are often
	 * retrieved as a pre-cursor to rendering and there's almost zero
	 * time overhead to caching the image (just a space one) and this
	 * saves re-scaling the glyph to get the image. The primary space
	 * concern is alleviated by doing this only for strikes which are
	 * smaller point sizes and address typical on-screen cases where
	 * re-use is likely and performance is most important.
	 */
	if (getImageWithAdvance && numGlyphs < MAXADVANCECACHESIZE) {
	    horizontalAdvances = new float[numGlyphs];
	    /* use max float as uninitialised advance */
	    for (int i=0; i<numGlyphs; i++) {
		horizontalAdvances[i] = Float.MAX_VALUE;
	    }
	}

    }

    /* The asymmetry of the following methods is to help preserve
     * performance with minimal textual changes to the calling code
     * when moving initialisation of these arrays out of the constructor.
     * This may be restructured later when there's more room for changes
     */
    private boolean usingIntGlyphImages() {
	if (intGlyphImages != null) {
	    return true;
	} else if (FontManager.longAddresses) {
	    return false;
	} else {
	    intGlyphImages = new int[mapper.getNumGlyphs()];
	    this.disposer.intGlyphImages = intGlyphImages;
	    return true;
	}
    }

    private long[] getLongGlyphImages() {
	if (longGlyphImages == null && FontManager.longAddresses) {
	    longGlyphImages = new long[mapper.getNumGlyphs()];
	    this.disposer.longGlyphImages = longGlyphImages;
	}
	return longGlyphImages;
    }

   /* Retrieves a singleton "null" scaler context instance which must
     * not be freed.
     */
    static synchronized native long getNullScalerContext(long pScaler);

    private native long createScalerContext(long pScaler, double[] matrix,
					    boolean fontType,
					    boolean aa, boolean fm, 
					    boolean algStyle,
					    float boldness, float italic);

    /* A number of methods are delegated by the strike to the scaler
     * context which is a shared resource on a physical font.
     */

    public int getNumGlyphs() {
	return fileFont.getNumGlyphs();
    }

    /* Try the native strikes first, then try the fileFont strike */
    long getGlyphImageFromNative(int glyphCode) {
	long glyphPtr;
	char charCode = fileFont.glyphToCharMap[glyphCode];
	for (int i=0;i<nativeStrikes.length;i++) {
	    CharToGlyphMapper mapper = fileFont.nativeFonts[i].getMapper();
	    int gc = mapper.charToGlyph(charCode)&0xffff;
	    if (gc != mapper.getMissingGlyphCode()) {
		glyphPtr = nativeStrikes[i].getGlyphImagePtrNoCache(gc);
		if (glyphPtr != 0L) {
		    return glyphPtr;
		}
	    }
	}
        return fileFont.getGlyphImage(pScalerContext, glyphCode);
    }

    /* Is the "if" and "mask" enough to warrant creating intaddress
     * and longaddress "pipes"?
     * This function is complicated but the path through should be fairly
     * short for all but the exceptional cases.
     */
    long getGlyphImagePtr(int glyphCode) {
	if (glyphCode >= INVISIBLE_GLYPHS) {
            return StrikeCache.invisibleGlyphPtr;
	}
	long glyphPtr;
	if (usingIntGlyphImages()) {
	    if ((glyphPtr = intGlyphImages[glyphCode] & INTMASK) != 0L) {
		return glyphPtr;
	    } else {
		if (useNatives) {
		    glyphPtr = getGlyphImageFromNative(glyphCode);
		} else {
		    glyphPtr = fileFont.getGlyphImage(pScalerContext,
						      glyphCode);
		}
		/* Synchronize in case some other thread has updated this
		 * cache entry already - unlikely but possible.
		 */
		synchronized (this) {
		    if (intGlyphImages[glyphCode] == 0) {
			intGlyphImages[glyphCode] = (int)glyphPtr;
			return glyphPtr;
		    } else {
			StrikeCache.freeIntPointer((int)glyphPtr);
			return intGlyphImages[glyphCode] & INTMASK;
		    }
		}
	    }
	} 
	/* must be using long (8 byte) addresses */
 	else if ((glyphPtr = getLongGlyphImages()[glyphCode]) != 0L) {
	    return glyphPtr;
	} else {
	    if (useNatives) {
		glyphPtr = getGlyphImageFromNative(glyphCode);
	    } else {
		glyphPtr = fileFont.getGlyphImage(pScalerContext,
						  glyphCode);
	    }
	    synchronized (this) {
		if (longGlyphImages[glyphCode] == 0L) {
		    longGlyphImages[glyphCode] = glyphPtr;
		    return glyphPtr;
		} else {  
		    StrikeCache.freeLongPointer(glyphPtr);
		    return longGlyphImages[glyphCode];
		}
	    }
	}
    }

    void getGlyphImagePtrs(int[] glyphCodes, long[] images, int  len) {

	long glyphPtr;
	if (usingIntGlyphImages()) {
	    for (int i=0; i<len; i++) {
		int glyphCode = glyphCodes[i];
		if (glyphCode >= INVISIBLE_GLYPHS) {
                    images[i] = StrikeCache.invisibleGlyphPtr;
                    continue;
		}
		if ((images[i] = intGlyphImages[glyphCode] & INTMASK)
		    != 0L) {
		    continue;
		} else {
		    if (useNatives) {
			glyphPtr = getGlyphImageFromNative(glyphCode);
		    } else {
			glyphPtr = fileFont.getGlyphImage(pScalerContext,
							  glyphCode);
		    }
		    /* Synchronize in case some other thread has updated this
		     * cache entry already - unlikely but possible.
		     */
		    synchronized (this) {
			if (intGlyphImages[glyphCode] == 0) {
			    intGlyphImages[glyphCode] = (int)glyphPtr;
			    images[i] = glyphPtr;
			} else {
			    StrikeCache.freeIntPointer((int)glyphPtr);
			    images[i]= intGlyphImages[glyphCode] & INTMASK;
			}
		    }
		}
	    }
	} else {  /* must be using long (8 byte) addresses */
	    long[] longGlyphImages = getLongGlyphImages();
	    for (int i=0; i<len; i++) {
		int glyphCode = glyphCodes[i];
		if (glyphCode >= INVISIBLE_GLYPHS) {
                    images[i] = StrikeCache.invisibleGlyphPtr;
                    continue;
		}
		if ((images[i] = longGlyphImages[glyphCode]) != 0L) {
		    continue;
		} else {
		    if (useNatives) {
			glyphPtr = getGlyphImageFromNative(glyphCode);
		    } else {
			glyphPtr = fileFont.getGlyphImage(pScalerContext,
							  glyphCode);
		    }

		    synchronized (this) {
			if (longGlyphImages[glyphCode] == 0L) {
			    longGlyphImages[glyphCode] = glyphPtr;
			    images[i] = glyphPtr;
			} else {  
			    StrikeCache.freeLongPointer(glyphPtr);
			    images[i] = longGlyphImages[glyphCode];
			}
		    }
		}
	    }
	}
    }

    /* The following method is called from CompositeStrike as a special case.
     */
    private static final int SLOTZEROMAX = 0xffffff;
    int getSlot0GlyphImagePtrs(int[] glyphCodes, long[] images, int  len) {

	int convertedCnt = 0;
	int glyphCode;
	long glyphPtr;
	if (usingIntGlyphImages()) {
	    for (int i=0; i<len; i++) {
		glyphCode = glyphCodes[i];
		if (glyphCode >= SLOTZEROMAX) {
		    return convertedCnt;
		} else {
		    convertedCnt++;
		}
		if (glyphCode >= INVISIBLE_GLYPHS) {
		    images[i] = StrikeCache.invisibleGlyphPtr;
		    continue;
		}
		if ((images[i] = intGlyphImages[glyphCode] & INTMASK) != 0L) {
		    continue;
		} else {
		    if (useNatives) {
			glyphPtr = getGlyphImageFromNative(glyphCode);
		    } else {
			glyphPtr = fileFont.getGlyphImage(pScalerContext,
							  glyphCode);
		    }
		    /* Synchronize in case some other thread has updated this
		     * cache entry already - unlikely but possible.
		     */
		    synchronized (this) {
			if (intGlyphImages[glyphCode] == 0) {
			    intGlyphImages[glyphCode] = (int)glyphPtr;
			    images[i] = glyphPtr;
			} else {
			    StrikeCache.freeIntPointer((int)glyphPtr);
			    images[i]= intGlyphImages[glyphCode] & INTMASK;
			}
		    }
		}
	    }
	} else {  /* must be using long (8 byte) addresses */
	    long[] longGlyphImages = getLongGlyphImages();
	    for (int i=0; i<len; i++) {
		glyphCode = glyphCodes[i];

		if (glyphCode >= SLOTZEROMAX) {
		    return convertedCnt;
		} else {
		    convertedCnt++;
		}
		if (glyphCode >= INVISIBLE_GLYPHS) {
		    images[i] = StrikeCache.invisibleGlyphPtr;
		    continue;
		}
		if ((images[i] = longGlyphImages[glyphCode]) != 0L) {
		    continue;
		} else {
		    glyphPtr = fileFont.getGlyphImage(pScalerContext,
						      glyphCode);

		    synchronized (this) {
			if (longGlyphImages[glyphCode] == 0L) {
			    longGlyphImages[glyphCode] = glyphPtr;
			    images[i] = glyphPtr;
			} else {  
			    StrikeCache.freeLongPointer(glyphPtr);
			    images[i] = longGlyphImages[glyphCode];
			}
		    }
		}
	    }
	}
	return convertedCnt;
    }

    /* Only look in the cache */
    long getCachedGlyphPtr(int glyphCode) {
	if (intGlyphImages != null) {
	    return intGlyphImages[glyphCode] & INTMASK;
	} else if (longGlyphImages != null) {
	    return longGlyphImages[glyphCode];
	} else {
	    return 0L;
	}
    }

    /* Metrics info is always retrieved. If the GlyphInfo address is non-zero
     * then metrics info there is valid and can just be copied.
     * This is in user space coordinates.
     */
    float getGlyphAdvance(int glyphCode) {

	float advance;

	if (glyphCode >= INVISIBLE_GLYPHS) {
	    return 0f;
	}
	if (horizontalAdvances != null) {
	    advance = horizontalAdvances[glyphCode];
	    if (advance != Float.MAX_VALUE) {
		return advance;
	    }
	}

	if (invertDevTx != null) {
	    /* If there is a device transform need x & y advance to
	     * transform back into user space.
	     */
	    advance = getGlyphMetrics(glyphCode).x;
	} else {
	    long glyphPtr;
	    if (getImageWithAdvance) {
		/* A heuristic optimisation says that for some cases its
		 * worthwhile retrieving the image at the same time as the
		 * advance. So here we get the image data even if its not
		 * already cached.
		 */	    
		glyphPtr = getGlyphImagePtr(glyphCode);
	    } else {
		glyphPtr = getCachedGlyphPtr(glyphCode);
	    }
	    if (glyphPtr != 0L) {
		advance = StrikeCache.unsafe.getFloat
		    (glyphPtr + StrikeCache.xAdvanceOffset);

	    } else {
		advance = fileFont.getGlyphAdvance(pScalerContext, glyphCode);
	    }
	}

	if (horizontalAdvances != null) {
	    horizontalAdvances[glyphCode] = advance;
	}
	return advance;
    }

    float getCodePointAdvance(int cp) {
	return getGlyphAdvance(mapper.charToGlyph(cp));
    }

    /**
     * Result and pt are both in device space.
     */
    void getGlyphImageBounds(int glyphCode, Point2D.Float pt,
			     Rectangle result) {

        long ptr = getGlyphImagePtr(glyphCode);
	float topLeftX =
	  StrikeCache.unsafe.getFloat(ptr+StrikeCache.topLeftXOffset);
	float topLeftY =
	  StrikeCache.unsafe.getFloat(ptr+StrikeCache.topLeftYOffset);

	result.x = (int)Math.floor(pt.x + topLeftX);
	result.y = (int)Math.floor(pt.y + topLeftY);
        result.width =
	    StrikeCache.unsafe.getShort(ptr+StrikeCache.widthOffset)  &0x0ffff;
        result.height =
	    StrikeCache.unsafe.getShort(ptr+StrikeCache.heightOffset) &0x0ffff;
    }


    /* These 3 metrics methods below should be implemented to return
     * values in user space.
     */
    StrikeMetrics getFontMetrics() {
	if (strikeMetrics == null) {
	    strikeMetrics =
		fileFont.getFontMetrics(pScalerContext);
	    if (invertDevTx != null) {
		strikeMetrics.convertToUserSpace(invertDevTx);
	    }
	}
	return strikeMetrics;
    }

    Point2D.Float getGlyphMetrics(int glyphCode) {
	Point2D.Float metrics = new Point2D.Float();

	// !!! or do we force sgv user glyphs?
	if (glyphCode >= INVISIBLE_GLYPHS) {
	    return metrics;
        }
	long glyphPtr;
	if (getImageWithAdvance) {
	    /* A heuristic optimisation says that for some cases its
	     * worthwhile retrieving the image at the same time as the
	     * metrics. So here we get the image data even if its not
	     * already cached.
	     */
	    glyphPtr = getGlyphImagePtr(glyphCode);
	} else {
	     glyphPtr = getCachedGlyphPtr(glyphCode);
	}
	if (glyphPtr != 0L) {
	    metrics = new Point2D.Float();
	    metrics.x = StrikeCache.unsafe.getFloat
		(glyphPtr + StrikeCache.xAdvanceOffset);
	    metrics.y = StrikeCache.unsafe.getFloat
		(glyphPtr + StrikeCache.yAdvanceOffset);
	    /* advance is currently in device space, need to convert back
	     * into user space.
	     * This must not include the translation component. */
	    if (invertDevTx != null) {
		invertDevTx.deltaTransform(metrics, metrics);
	    }
	} else {
	    /* We sometimes cache these metrics as they are expensive to 
	     * generate for large glyphs.
	     * We never reach this path if we obtain images with advances.
	     * But if we do not obtain images with advances its possible that
	     * we first obtain this information, then the image, and never
	     * will access this value again.
	     */
	    Integer key = new Integer(glyphCode);
	    Point2D.Float value = null;
	    HashMap glyphMetricsMap = null;
	    if (glyphMetricsMapRef != null) {
	        glyphMetricsMap = (HashMap)glyphMetricsMapRef.get();
	    }
	    if (glyphMetricsMap != null) {
		synchronized (this) {
		    value = (Point2D.Float)glyphMetricsMap.get(key);
		    if (value != null) {
			metrics.x = value.x;
			metrics.y = value.y;
			/* already in user space */
			return metrics;
		    }
		}
	    }
	    if (value == null) {
		fileFont.getGlyphMetrics(pScalerContext, glyphCode, metrics);
		/* advance is currently in device space, need to convert back
		 * into user space.
		 */
		if (invertDevTx != null) {
		    invertDevTx.deltaTransform(metrics, metrics);
		}		
	        value = new Point2D.Float(metrics.x, metrics.y);
		synchronized (this) {
		    if (glyphMetricsMap == null) {
			glyphMetricsMap = new HashMap();
			glyphMetricsMapRef= new SoftReference(glyphMetricsMap);
		    }
		    glyphMetricsMap.put(key, value);
		}
	    }
	}
	return metrics;
    }

    Point2D.Float getCharMetrics(char ch) {
	return getGlyphMetrics(mapper.charToGlyph(ch));
    }

    /* The caller of this can be trusted to return a copy of this
     * return value rectangle to public API. In fact frequently it
     * can't use use this return value directly anyway.
     * This returns bounds in device space. Currently the only
     * caller is SGV and it converts back to user space.
     * We could change things so that this code does the conversion so
     * that all coords coming out of the font system are converted back
     * into user space even if they were measured in device space.
     * The same applies to the other methods that return outlines (below)
     * But it may make particular sense for this method that caches its
     * results.
     * There'd be plenty of exceptions, to this too, eg getGlyphPoint needs
     * device coords as its called from native layout and getGlyphImageBounds
     * is used by GlyphVector.getGlyphPixelBounds which is specified to
     * return device coordinates, the image pointers aren't really used
     * up in Java code either.
     */
    Rectangle2D.Float getGlyphOutlineBounds(int glyphCode) {
	Object key = new Integer(glyphCode);
	Rectangle2D.Float bounds;

	if (boundsMap == null) {
	    synchronized (this) {
		if (boundsMap == null) {
		    boundsMap = new HashMap();
		}
	    }
	}
	synchronized (boundsMap) {
	    bounds = (Rectangle2D.Float)boundsMap.get(key);
	}

	if (bounds == null) {
	    bounds = fileFont.getGlyphOutlineBounds(pScalerContext, glyphCode);
	    synchronized (boundsMap) {
		boundsMap.put(key, bounds);
	    }
	}
	return bounds;
    }

    public Rectangle2D getOutlineBounds(int glyphCode) {
	return fileFont.getGlyphOutlineBounds(pScalerContext, glyphCode);
    }

    GeneralPath getGlyphOutline(int glyphCode, float x, float y) {
	return fileFont.getGlyphOutline(pScalerContext,	glyphCode, x, y);
    }

    GeneralPath getGlyphVectorOutline(int[] glyphs, float x, float y) {
	return fileFont.getGlyphVectorOutline(pScalerContext,
					      glyphs, glyphs.length, x, y);
    }

    protected void adjustPoint(Point2D.Float pt) {
	if (invertDevTx != null) {
      	    invertDevTx.deltaTransform(pt, pt);
	}
    }
}
