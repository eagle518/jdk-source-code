/*
 * @(#)SunGraphics2D.java	1.332 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.RenderingHints.Key;
import java.awt.geom.Area;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.RenderedImage;
import java.awt.image.renderable.RenderableImage;
import java.awt.image.renderable.RenderContext;
import java.awt.image.AffineTransformOp;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import java.awt.Image;
import java.awt.Composite;
import java.awt.Color;
import java.awt.color.ColorSpace;
import java.awt.image.DataBuffer;
import java.awt.image.ColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.DirectColorModel;
import java.awt.GraphicsConfiguration;
import java.awt.Paint;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.PathIterator;
import java.awt.geom.GeneralPath;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.FontMetrics;
import java.awt.Rectangle;
import java.text.AttributedCharacterIterator;
import java.awt.Font;
import java.awt.image.ImageObserver;
import java.awt.image.ColorConvertOp;
import java.awt.Transparency;
import java.awt.font.GlyphVector;
import java.awt.font.TextLayout;
import sun.font.FontDesignMetrics;
import sun.font.StandardGlyphVector;
import sun.java2d.pipe.PixelDrawPipe;
import sun.java2d.pipe.PixelFillPipe;
import sun.java2d.pipe.ShapeDrawPipe;
import sun.java2d.pipe.ValidatePipe;
import sun.java2d.pipe.ShapeSpanIterator;
import sun.java2d.pipe.Region;
import sun.java2d.pipe.RegionIterator;
import sun.java2d.pipe.TextPipe;
import sun.java2d.pipe.DrawImagePipe;
import sun.java2d.pipe.DuctusRenderer;
import sun.java2d.loops.FontInfo;
import sun.java2d.loops.RenderLoops;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.Blit;
import sun.java2d.loops.BlitBg;
import sun.java2d.loops.MaskFill;
import sun.font.FontManager;
import java.awt.font.FontRenderContext;
import sun.java2d.loops.XORComposite;
import sun.awt.ConstrainableGraphics;
import sun.awt.SunHints;
import java.util.Map;
import java.util.Iterator;
import sun.awt.image.OffScreenImage;
import sun.misc.PerformanceLogger;

/**
 * This is a the master Graphics2D superclass for all of the Sun
 * Graphics implementations.  This class relies on subclasses to
 * manage the various device information, but provides an overall
 * general framework for performing all of the requests in the
 * Graphics and Graphics2D APIs.
 *
 * @version 1.211 05/07/98
 * @author Jim Graham
 */
public final class SunGraphics2D
    extends Graphics2D
    implements ConstrainableGraphics, Cloneable
{
    public static final ColorModel XRGBModel =
        new DirectColorModel(24,0x00ff0000,0x0000ff00,0x000000ff);

    /*
     * Attribute States
     */
    /* Paint */
    public static final int PAINT_TILE        = 2; /* delivered as Tile */
    public static final int PAINT_SINGLECOLOR = 1; /* Single Color */
    public static final int PAINT_SOLIDCOLOR  = 0; /* Solid single Color */

    /* Composite*/
    public static final int COMP_CUSTOM = 3;/* Custom Composite */
    public static final int COMP_XOR    = 2;/* XOR Mode Composite */
    public static final int COMP_ALPHA  = 1;/* AlphaComposite */
    public static final int COMP_ISCOPY = 0;/* SRC, extraAlpha = 1.0 or
					     * SRC_OVER, extraAlpha = 1.0 */

    /* Stroke */
    public static final int STROKE_CUSTOM = 3; /* custom Stroke */
    public static final int STROKE_WIDE   = 2; /* BasicStroke */
    public static final int STROKE_THINDASHED   = 1; /* BasicStroke */
    public static final int STROKE_THIN   = 0; /* BasicStroke */

    /* Transform */
    public static final int TRANSFORM_GENERIC = 4; /* any 3x2 */
    public static final int TRANSFORM_TRANSLATESCALE = 3; /* scale XY */
    public static final int TRANSFORM_ANY_TRANSLATE = 2; /* non-int translate */
    public static final int TRANSFORM_INT_TRANSLATE = 1; /* int translate */
    public static final int TRANSFORM_ISIDENT = 0; /* Identity */

    /* Clipping */
    public static final int CLIP_SHAPE       = 2; /* arbitrary clip */
    public static final int CLIP_RECTANGULAR = 1; /* rectangular clip */
    public static final int CLIP_DEVICE      = 0; /* no clipping set */

    public int rgb;
    public int pixel;

    public SurfaceData surfaceData;

    public PixelDrawPipe drawpipe;
    public PixelFillPipe fillpipe;
    public DrawImagePipe imagepipe;
    public ShapeDrawPipe shapepipe;
    public TextPipe textpipe;
    public MaskFill alphafill;

    public RenderLoops loops;

    public CompositeType fillComp;      /* Considering Paint Transparency */
    public CompositeType imageComp;     /* Image Transparency checked on fly */

    public int paintState;
    public int compositeState;
    public int strokeState;
    public int transformState;
    public int clipState;

    public Color foregroundColor;
    public Color backgroundColor;

    public AffineTransform transform;
    public int transX;
    public int transY;

    protected static final Stroke defaultStroke = new BasicStroke();
    protected static final Composite defaultComposite = AlphaComposite.SrcOver;
    private static final Font defaultFont = new Font("Dialog", Font.PLAIN, 12);

    public Paint paint;
    public Stroke stroke;
    public Composite composite;
    protected Font font;
    protected FontMetrics fontMetrics;

    public int renderHint;
    public int antialiasHint;
    public int textAntialiasHint;
    private int fractionalMetricsHint;
    private int interpolationHint;	// raw value of rendering Hint
    public int strokeHint;

    public int interpolationType;	// algorithm choice based on
					// interpolation and render Hints

    public RenderingHints hints;

    public Region constrainClip;		// lightweight bounds
    public int constrainX;
    public int constrainY;

    public Region clipRegion;
    public Shape usrClip;
    protected Region devClip;		// Actual physical drawable

    // cached state for text rendering
    private boolean validFontInfo;
    private FontInfo fontInfo;
    public  FontInfo glyphVectorFontInfo;

    private final static int slowTextTransformMask =
                            AffineTransform.TYPE_GENERAL_TRANSFORM
                        |   AffineTransform.TYPE_MASK_ROTATION
                        |   AffineTransform.TYPE_FLIP;

    static {
	if (PerformanceLogger.loggingEnabled()) {
	    PerformanceLogger.setTime("SunGraphics2D static initialization");
	}
    }

    public SunGraphics2D(SurfaceData sd, Color fg, Color bg, Font f) {
	foregroundColor = fg;
	backgroundColor = bg;
	int rgb;
	this.rgb = rgb = fg.getRGB();
	pixel = sd.pixelFor(rgb);

	transform = new AffineTransform();
	stroke = defaultStroke;
	composite = defaultComposite;
	paint = foregroundColor;

	fillComp = CompositeType.SrcNoEa;
	imageComp = CompositeType.SrcOverNoEa;

	renderHint = SunHints.INTVAL_RENDER_DEFAULT;
	antialiasHint = SunHints.INTVAL_ANTIALIAS_OFF;
	textAntialiasHint = SunHints.INTVAL_TEXT_ANTIALIAS_DEFAULT;
	fractionalMetricsHint = SunHints.INTVAL_FRACTIONALMETRICS_OFF;
	interpolationHint = -1;
	strokeHint = SunHints.INTVAL_STROKE_DEFAULT;

	interpolationType = AffineTransformOp.TYPE_NEAREST_NEIGHBOR;

	font = f;
	if (font == null) {
	    font = defaultFont;
	}

	surfaceData = sd;
	loops = sd.getRenderLoops(this);
	setDevClip(sd.getBounds());
	invalidatePipe();
    }

    protected Object clone() {
	try {
	    SunGraphics2D g = (SunGraphics2D) super.clone();
	    g.transform = new AffineTransform(this.transform);
	    if (hints != null) {
		g.hints = (RenderingHints) this.hints.clone();
	    }
	    /* FontInfos are re-used, so must be cloned too, if they
	     * are valid, and be nulled out if invalid.
	     * The implied trade-off is that there is more to be gained
	     * from re-using these objects than is lost by having to
	     * clone them when the SG2D is cloned.
	     */
	    if (this.fontInfo != null) {
		if (this.validFontInfo) {
		    g.fontInfo = (FontInfo)this.fontInfo.clone();
		} else {
		    g.fontInfo = null;
		}
	    }
	    if (this.glyphVectorFontInfo != null) {
		g.glyphVectorFontInfo =
		    (FontInfo)this.glyphVectorFontInfo.clone();
	    }
	    //g.invalidatePipe();
	    return g;
	} catch (CloneNotSupportedException e) {
	}
	return null;
    }

    /**
     * Create a new SunGraphics2D based on this one.
     */
    public Graphics create() {
	return (Graphics) clone();
    }

    public void setDevClip(int x, int y, int w, int h) {
	Region c = constrainClip;
	if (c == null) {
	    devClip = Region.getInstanceXYWH(x, y, w, h);
	} else {
	    devClip = c.getIntersectionXYWH(x, y, w, h);
	}
	invalidateClip();
    }

    public void setDevClip(Rectangle r) {
	setDevClip(r.x, r.y, r.width, r.height);
    }

    /**
     * Constrain rendering for lightweight objects.
     *
     * REMIND: This method will back off to the "workaround"
     * of using translate and clipRect if the Graphics
     * to be constrained has a complex transform.  The
     * drawback of the workaround is that the resulting
     * clip and device origin cannot be "enforced".
     *
     * @exception IllegalStateException If the Graphics
     * to be constrained has a complex transform.
     */
    public void constrain(int x, int y, int w, int h) {
        if ((x|y) != 0) {
            translate(x, y);
        }
	if (transformState >= TRANSFORM_TRANSLATESCALE) {
	    clipRect(0, 0, w, h);
	    return;
	}
        x = constrainX = transX;
        y = constrainY = transY;
	w = Region.dimAdd(x, w);
	h = Region.dimAdd(y, h);
	Region c = constrainClip;
	if (c == null) {
	    c = Region.getInstanceXYXY(x, y, w, h);
	} else {
	    c = c.getIntersectionXYXY(x, y, w, h);
	    if (c == constrainClip) {
		// Common case to ignore
		return;
	    }
	}
	constrainClip = c;
	if (!devClip.isInsideQuickCheck(c)) {
	    devClip = devClip.getIntersection(c);
	    invalidateClip();
	}
    }

    protected static ValidatePipe invalidpipe = new ValidatePipe();

    /*
     * Invalidate the pipeline
     */
    protected void invalidatePipe() {
	drawpipe = invalidpipe;
	fillpipe = invalidpipe;
	shapepipe = invalidpipe;
	textpipe = invalidpipe;
	imagepipe = invalidpipe;
    }

    public void validatePipe() {
	surfaceData.validatePipe(this);
    }

    /*
     * Intersect two Shapes by the simplest method, attempting to produce
     * a simplified result.
     * The boolean arguments keep1 and keep2 specify whether or not
     * the first or second shapes can be modified during the operation
     * or whether that shape must be "kept" unmodified.
     */
    Shape intersectShapes(Shape s1, Shape s2, boolean keep1, boolean keep2) {
	if (s1 instanceof Rectangle && s2 instanceof Rectangle) {
	    return ((Rectangle) s1).intersection((Rectangle) s2);
	}
	if (s1 instanceof Rectangle2D) {
	    return intersectRectShape((Rectangle2D) s1, s2, keep1, keep2);
	} else if (s2 instanceof Rectangle2D) {
	    return intersectRectShape((Rectangle2D) s2, s1, keep2, keep1);
	}
	return intersectByArea(s1, s2, keep1, keep2);
    }

    /*
     * Intersect a Rectangle with a Shape by the simplest method,
     * attempting to produce a simplified result.
     * The boolean arguments keep1 and keep2 specify whether or not
     * the first or second shapes can be modified during the operation
     * or whether that shape must be "kept" unmodified.
     */
    Shape intersectRectShape(Rectangle2D r, Shape s,
			     boolean keep1, boolean keep2) {
	if (s instanceof Rectangle2D) {
	    Rectangle2D r2 = (Rectangle2D) s;
	    Rectangle2D outrect;
	    if (!keep1) {
		outrect = r;
	    } else if (!keep2) {
		outrect = r2;
	    } else {
		outrect = new Rectangle2D.Float();
	    }
	    double x1 = Math.max(r.getX(), r2.getX());
	    double x2 = Math.min(r.getX()  + r.getWidth(), 
                                 r2.getX() + r2.getWidth());
	    double y1 = Math.max(r.getY(), r2.getY());
	    double y2 = Math.min(r.getY()  + r.getHeight(), 
                                 r2.getY() + r2.getHeight());

            if (((x2 - x1) < 0) || ((y2 - y1) < 0))
                // Width or height is negative. No intersection.
		outrect.setFrameFromDiagonal(0, 0, 0, 0);
            else
		outrect.setFrameFromDiagonal(x1, y1, x2, y2);
	    return outrect;
	}
	if (r.contains(s.getBounds2D())) {
	    if (keep2) {
		s = cloneShape(s);
	    }
	    return s;
	}
	return intersectByArea(r, s, keep1, keep2);
    }

    protected static Shape cloneShape(Shape s) {
	return new GeneralPath(s);
    }
    
    /*
     * Intersect two Shapes using the Area class.  Presumably other
     * attempts at simpler intersection methods proved fruitless.
     * The boolean arguments keep1 and keep2 specify whether or not
     * the first or second shapes can be modified during the operation
     * or whether that shape must be "kept" unmodified.
     * @see #intersectShapes
     * @see #intersectRectShape
     */
    Shape intersectByArea(Shape s1, Shape s2, boolean keep1, boolean keep2) {
	Area a1, a2;

	// First see if we can find an overwriteable source shape
	// to use as our destination area to avoid duplication.
	if (!keep1 && (s1 instanceof Area)) {
	    a1 = (Area) s1;
	} else if (!keep2 && (s2 instanceof Area)) {
	    a1 = (Area) s2;
	    s2 = s1;
	} else {
	    a1 = new Area(s1);
	}

	if (s2 instanceof Area) {
	    a2 = (Area) s2;
	} else {
	    a2 = new Area(s2);
	}

	a1.intersect(a2);
	if (a1.isRectangular()) {
	    return a1.getBounds();
	}

	return a1;
    }

    /*
     * Intersect usrClip bounds and device bounds to determine the composite
     * rendering boundaries
     */
    public Region getCompClip() {
	if (!surfaceData.isValid()) {
	    revalidateAll();
	}

	if (clipRegion == null) {
	    switch (clipState) {
	    case CLIP_DEVICE:
		clipRegion = devClip;
		break;
	    case CLIP_RECTANGULAR:
		if (usrClip instanceof Rectangle) {
		    clipRegion = devClip.getIntersection((Rectangle) usrClip);
		} else {
		    clipRegion = devClip.getIntersection(usrClip.getBounds());
		}
		break;
	    case CLIP_SHAPE:
		PathIterator cpi = usrClip.getPathIterator(null);
		int box[] = new int[4];

		ShapeSpanIterator sr = new ShapeSpanIterator(this, false);
		try {
		    sr.setOutputArea(devClip);
		    sr.appendPath(cpi);
		    sr.getPathBox(box);
		    Region r = Region.getInstance(box);
		    r.appendSpans(sr);
		    clipRegion = r;
		    if (r.isRectangular()) {
			clipState = CLIP_RECTANGULAR;
		    }
		} finally {
		    sr.dispose();
		}
		break;
	    }
	}

	return clipRegion;
    }
    
    /*
     * Convert a BufferedImage into another BufferedImage with a desired
     * ColorModel.
     */
    protected BufferedImage convertCM(BufferedImage src, ColorModel cm) {
        WritableRaster wr = 
              cm.createCompatibleWritableRaster(src.getWidth(), 
                                                 src.getHeight());
        BufferedImage dst = new BufferedImage(cm, wr,
                                              src.isAlphaPremultiplied(),
                                              null);
        for (int i = 0 ; i < src.getHeight() ; i++) {
            for (int j = 0 ; j < src.getWidth() ; j++) {
                dst.setRGB(j, i, src.getRGB(j, i));
            }
        }
        return dst;
    }

    /*
     * Convert a given Raster to the desired data format.
     */
    public WritableRaster convertRaster(Raster inRaster, 
                                         ColorModel inCM, 
                                         ColorModel outCM) {
        // Use a faster conversion if this is an IndexColorModel
        if (inCM instanceof IndexColorModel &&
            ((outCM.equals(ColorModel.getRGBdefault())) ||
             (outCM.equals(XRGBModel)))) {
            IndexColorModel icm = (IndexColorModel) inCM;
            BufferedImage dbi = icm.convertToIntDiscrete(inRaster, false);
            return dbi.getRaster();
        }

        BufferedImage dbi =
            new BufferedImage(outCM,
                    outCM.createCompatibleWritableRaster(inRaster.getWidth(),
                              inRaster.getHeight()),
                              outCM.isAlphaPremultiplied(),
                              null);
        
        //     ColorSpace[] cs = {inCM.getColorSpace(), outCM.getColorSpace()};
        //     ColorConvertOp cOp = new ColorConvertOp(cs);
        //     cOp.filter(sbi, dbi);

        // use this slow method to convert untill ColorConvertOp is available.
        // Does not take in to account quality dithering if applicable.
        Object buffer = inRaster.getDataElements(0, 0, null);
        for (int i = 0 ; i < dbi.getHeight() ; i++) {
            for (int j = 0 ; j < dbi.getWidth() ; j++) {
                dbi.setRGB(j, i, inCM.getRGB(inRaster.getDataElements(j,i,buffer)));
            }
        }

        return dbi.getRaster();
    }

    public Font getFont() {
        if (font == null) {
            font = defaultFont;
        }
        return font;
    }

    private static final double[] IDENT_MATRIX = {1, 0, 0, 1};
    private static final AffineTransform IDENT_ATX =
	new AffineTransform();

    private static final int MINALLOCATED = 8;
    private static final int TEXTARRSIZE = 17;
    private static double[][] textTxArr = new double[TEXTARRSIZE][];
    private static AffineTransform[] textAtArr =
	new AffineTransform[TEXTARRSIZE];

    static {
	for (int i=MINALLOCATED;i<TEXTARRSIZE;i++) {
	  textTxArr[i] = new double [] {i, 0, 0, i};
	  textAtArr[i] = new AffineTransform( textTxArr[i]);
	}
    }

    // cached state for various draw[String,Char,Byte] optimizations
    public FontInfo checkFontInfo(FontInfo info, Font font) {
	/* Do not create a FontInfo object as part of construction of an
	 * SG2D as its possible it may never be needed - ie if no text
	 * is drawn using this SG2D.
	 */
	if (info == null) {
	    info = new FontInfo();
	}
  
	float ptSize = font.getSize2D();
	int txFontType;
	AffineTransform devAt, textAt=null;
	if (font.isTransformed()) {
	    textAt = font.getTransform();
	    textAt.scale(ptSize, ptSize);
	    txFontType = textAt.getType();
	    info.originX = (float)textAt.getTranslateX();
	    info.originY = (float)textAt.getTranslateY();
	    textAt.translate(-info.originX, -info.originY);
	    if (transformState >= TRANSFORM_TRANSLATESCALE) {
		transform.getMatrix(info.devTx = new double[4]);
		devAt = new AffineTransform(info.devTx);
		textAt.preConcatenate(devAt);
	    } else {
		info.devTx = IDENT_MATRIX;
		devAt = IDENT_ATX;
	    }
	    textAt.getMatrix(info.glyphTx = new double[4]);
	    info.pixelHeight = (int)(Math.abs(textAt.getScaleY()));
	} else {
	    txFontType = AffineTransform.TYPE_IDENTITY;
	    info.originX = info.originY = 0;
	    if (transformState >= TRANSFORM_TRANSLATESCALE) {
		transform.getMatrix(info.devTx = new double[4]);
		devAt = new AffineTransform(info.devTx);
		info.glyphTx = new double[4];
		for (int i = 0; i < 4; i++) {
		    info.glyphTx[i] = info.devTx[i] * ptSize;
		}
		textAt = new AffineTransform(info.glyphTx);
		info.pixelHeight = (int)(Math.abs(transform.getScaleY()
						  * ptSize));
	    } else {
		/* If the double represents a common integral, we
		 * may have pre-allocated objects.
		 * A "sparse" array be seems to be as fast as a switch
		 * even for 3 or 4 pt sizes, and is more flexible.
		 * This should perform comparably in single-threaded
		 * rendering to the old code which synchronized on the
		 * class and scale better on MP systems.
		 */
		int pszInt = (int)ptSize;
		if (ptSize == pszInt &&
		    pszInt >= MINALLOCATED && pszInt < TEXTARRSIZE) {
		    info.glyphTx = textTxArr[pszInt];
		    textAt = textAtArr[pszInt];
		}
		if (textAt == null) {
		    info.glyphTx = new double[] {ptSize, 0, 0, ptSize};
		    textAt = new AffineTransform(info.glyphTx);		    
		}

		info.pixelHeight = pszInt;
		info.devTx = IDENT_MATRIX;
		devAt = IDENT_ATX;
	    }
	}

	/* Currently these are being used only as args to getStrike,
	 * and are encapsulated in that object which is part of the
	 * FontInfo, so we do not need to store them directly as fields
	 * in the FontInfo object.
	 * That could change if FontInfo's were more selectively
	 * revalidated when graphics state changed. Presently this
	 * method re-evaluates all fields in the fontInfo.
	 */
	int aahint = textAntialiasHint;
	if (aahint == SunHints.INTVAL_TEXT_ANTIALIAS_DEFAULT) {
	    aahint = antialiasHint;
	}
	boolean doAntiAlias = (aahint == SunHints.INTVAL_ANTIALIAS_ON);
	boolean doFractEnable = (fractionalMetricsHint ==
				 SunHints.INTVAL_FRACTIONALMETRICS_ON);

	info.font2D = FontManager.getFont2D(font);
	info.fontStrike = info.font2D.getStrike(font, devAt, textAt,
						doAntiAlias, doFractEnable);
	return info;
    }

    public static boolean isRotated(double [] mtx) {
	if ((mtx[0] == mtx[3]) &&
	    (mtx[1] == 0.0) &&
            (mtx[2] == 0.0) &&
	    (mtx[0] > 0.0))
	{
            return false;
        }
 
        return true;
    }

    public void setFont(Font font) {
	/* replacing the reference equality test font != this.font with
	 * !font.equals(this.font) did not yield any measurable difference
	 * in testing, but there may be yet to be identified cases where it
	 * is beneficial.
	 */
        if (font != null && font!=this.font/*!font.equals(this.font)*/) {
            this.font = font;
            this.fontMetrics = null;
	    this.validFontInfo = false;
        }
    }

    public FontInfo getFontInfo() {
	if (!validFontInfo) {
	    this.fontInfo = checkFontInfo(this.fontInfo, font);
	    validFontInfo = true;
	}
        return this.fontInfo;
    }

    /* Used by drawGlyphVector which specifies its own font. */
    public FontInfo getGVFontInfo(Font font) {
	if (glyphVectorFontInfo != null &&
	    glyphVectorFontInfo.font == font) {
	    return glyphVectorFontInfo;
	} else {
	    return glyphVectorFontInfo =
		checkFontInfo(glyphVectorFontInfo, font);
	}
    }

    public static FontDesignMetrics metricsCache[] = new FontDesignMetrics[5];

    public static FontMetrics makeFontMetrics(Font font, FontRenderContext frc)
    {
	synchronized (metricsCache) {
	    for (int i = 0; i < metricsCache.length; i++) {
		FontDesignMetrics tfdm = metricsCache[i];
		if (tfdm == null) {
		    break;
		}
		if (tfdm.getFont().equals(font) && tfdm.getFRC().equals(frc)) {
		    if (i > 0 ) {
			System.arraycopy(metricsCache, 0, metricsCache, 1, i);
			metricsCache[0] = tfdm;
		    }
		    return tfdm;
		}
	    }
	}
	FontDesignMetrics fdm = new FontDesignMetrics(font, frc);
	synchronized (metricsCache) {
	    System.arraycopy(metricsCache, 0,
			     metricsCache, 1, metricsCache.length-1);
	    metricsCache[0] = fdm;
	}
	return fdm;
    }

    public FontMetrics getFontMetrics(Font font) {
        if ((this.fontMetrics != null) && (font == this.font)) {
            return this.fontMetrics;
        }
	FontMetrics fm;
	if (FontManager.usePlatformFontMetrics()) {
	    fm = java.awt.Toolkit.getDefaultToolkit().getFontMetrics(font);
	} else {
	    fm = makeFontMetrics(font, getFontRenderContext());
	}
        if (this.font == font) {
            this.fontMetrics = fm;
        }
        return fm;
    }

    /**
     * Checks to see if a Path intersects the specified Rectangle in device
     * space.  The rendering attributes taken into account include the
     * clip, transform, and stroke attributes.
     * @param rect The area in device space to check for a hit.
     * @param p The path to check for a hit.
     * @param onStroke Flag to choose between testing the stroked or
     * the filled path.
     * @return True if there is a hit, false otherwise.
     * @see #setStroke
     * @see #fillPath
     * @see #drawPath
     * @see #transform
     * @see #setTransform
     * @see #clip
     * @see #setClip
     */
    public boolean hit(Rectangle rect, Shape s, boolean onStroke) {
        if (onStroke) {
            s = stroke.createStrokedShape(s);
        }

        s = transformShape(s);
	if ((constrainX|constrainY) != 0) {
	    rect = new Rectangle(rect);
	    rect.translate(constrainX, constrainY);
	}

        return s.intersects(rect);
    }

    /**
     * Return the ColorModel associated with this Graphics2D.
     */
    public ColorModel getDeviceColorModel() {
	return surfaceData.getColorModel();
    }

    /**
     * Return the device configuration associated with this Graphics2D.
     */
    public GraphicsConfiguration getDeviceConfiguration() {
	return surfaceData.getDeviceConfiguration();
    }

    /**
     * Return the SurfaceData object assigned to manage the destination
     * drawable surface of this Graphics2D.
     */
    public final SurfaceData getSurfaceData() {
	return surfaceData;
    }

    /**
     * Sets the Composite in the current graphics state. Composite is used
     * in all drawing methods such as drawImage, drawString, drawPath,
     * and fillPath.  It specifies how new pixels are to be combined with
     * the existing pixels on the graphics device in the rendering process.
     * @param comp The Composite object to be used for drawing.
     * @see java.awt.Graphics#setXORMode
     * @see java.awt.Graphics#setPaintMode
     * @see AlphaComposite
     */
    public void setComposite(Composite comp) {
	if (composite == comp) {
	    return;
	}
	int newCompState;
	CompositeType newCompType;
        if (comp instanceof AlphaComposite) {
	    AlphaComposite alphacomp = (AlphaComposite) comp;
	    newCompType = CompositeType.forAlphaComposite(alphacomp);
	    imageComp = newCompType;
	    if (newCompType == CompositeType.SrcOverNoEa &&
		(paintState == PAINT_SOLIDCOLOR ||
		 (paintState == PAINT_TILE &&
		  paint.getTransparency() == Transparency.OPAQUE)))
	    {
		newCompType = CompositeType.SrcNoEa;
		newCompState = COMP_ISCOPY;
	    } else {
		// REMIND: Could handle Src this way too by munging the pixel
		if (newCompType == CompositeType.SrcNoEa) {
		    newCompState = COMP_ISCOPY;
		} else {
		    newCompState = COMP_ALPHA;
		}
	    }
        } else if (comp instanceof XORComposite) {
            newCompState = COMP_XOR;
	    newCompType = CompositeType.Xor;
	    imageComp = newCompType;
	} else if (comp == null) {
	    throw new IllegalArgumentException("null Composite");
        } else {
	    surfaceData.checkCustomComposite();
            newCompState = COMP_CUSTOM;
	    newCompType = CompositeType.General;
	    imageComp = newCompType;
        }
        if (compositeState != newCompState ||
	    fillComp != newCompType)
	{
	    compositeState = newCompState;
	    fillComp = newCompType;
	    alphafill = null;
	    invalidatePipe();
        }
	composite = comp;
    }

    /**
     * Sets the Paint in the current graphics state.
     * @param paint The Paint object to be used to generate color in
     * the rendering process.
     * @see java.awt.Graphics#setColor
     * @see GradientPaint
     * @see TexturePaint
     */
    public void setPaint(Paint paint) {
	if (paint instanceof Color) {
	    setColor((Color) paint);
	    return;
	}
	if (paint == null || this.paint == paint) {
	    return;
	}
        this.paint = paint;
	if (imageComp == CompositeType.SrcOverNoEa) {
	    // special case where fillComp depends on opacity of paint
	    if (paint.getTransparency() == Transparency.OPAQUE) {
		if (compositeState != COMP_ISCOPY) {
		    compositeState = COMP_ISCOPY;
		    fillComp = CompositeType.SrcNoEa;
		    alphafill = null;
		}
	    } else {
		if (compositeState == COMP_ISCOPY) {
		    compositeState = COMP_ALPHA;
		    fillComp = CompositeType.SrcOverNoEa;
		    alphafill = null;
		}
	    }
	}
        paintState = PAINT_TILE;
        invalidatePipe();
    }

    static final int NON_UNIFORM_SCALE_MASK =
	(AffineTransform.TYPE_GENERAL_TRANSFORM |
	 AffineTransform.TYPE_GENERAL_SCALE);
    public static final double MinPenSizeAASquared = 
	(DuctusRenderer.MinPenSizeAA * DuctusRenderer.MinPenSizeAA);
    // Since inaccuracies in the trig package can cause us to
    // calculated a rotated pen width of just slightly greater
    // than 1.0, we add a fudge factor to our comparison value
    // here so that we do not misclassify single width lines as
    // wide lines under certain rotations.
    public static final double MinPenSizeSquared = 1.000000001;

    private void validateBasicStroke(BasicStroke bs) {
	boolean aa = (antialiasHint == SunHints.INTVAL_ANTIALIAS_ON);
	if (transformState < TRANSFORM_TRANSLATESCALE) {
	    if (aa) {
		if (bs.getLineWidth() <= DuctusRenderer.MinPenSizeAA) {
		    if (bs.getDashArray() == null) {
			strokeState = STROKE_THIN;
		    } else {
			strokeState = STROKE_THINDASHED;
		    }
		} else {
		    strokeState = STROKE_WIDE;
		}
	    } else {
		if (bs == defaultStroke) {
		    strokeState = STROKE_THIN;
		} else if (bs.getLineWidth() <= 1.0f) {
		    if (bs.getDashArray() == null) {
			strokeState = STROKE_THIN;
		    } else {
			strokeState = STROKE_THINDASHED;
		    }
		} else {
		    strokeState = STROKE_WIDE;
		}
	    }
	} else {
	    double widthsquared;
	    if ((transform.getType() & NON_UNIFORM_SCALE_MASK) == 0) {
		/* sqrt omitted, compare to squared limits below. */
		widthsquared = Math.abs(transform.getDeterminant());
	    } else {
		/* First calculate the "maximum scale" of this transform. */
		double A = transform.getScaleX();	// m00
		double C = transform.getShearX();	// m01
		double B = transform.getShearY();	// m10
		double D = transform.getScaleY();	// m11

		/*
		 * Given a 2 x 2 affine matrix [ A B ] such that
		 *                             [ C D ]
		 * v' = [x' y'] = [Ax + Cy, Bx + Dy], we want to
		 * find the maximum magnitude (norm) of the vector v'
		 * with the constraint (x^2 + y^2 = 1).
		 * The equation to maximize is
		 *     |v'| = sqrt((Ax+Cy)^2+(Bx+Dy)^2)
		 * or  |v'| = sqrt((AA+BB)x^2 + 2(AC+BD)xy + (CC+DD)y^2).
		 * Since sqrt is monotonic we can maximize |v'|^2
		 * instead and plug in the substitution y = sqrt(1 - x^2).
		 * Trigonometric equalities can then be used to get
		 * rid of most of the sqrt terms.
		 */
		double EA = A*A + B*B;		// x^2 coefficient
		double EB = 2*(A*C + B*D);	// xy coefficient
		double EC = C*C + D*D;		// y^2 coefficient

		/*
		 * There is a lot of calculus omitted here.
		 *
		 * Conceptually, in the interests of understanding the
		 * terms that the calculus produced we can consider
		 * that EA and EC end up providing the lengths along
		 * the major axes and the hypot term ends up being an
		 * adjustment for the additional length along the off-axis
		 * angle of rotated or sheared ellipses as well as an
		 * adjustment for the fact that the equation below
		 * averages the two major axis lengths.  (Notice that
		 * the hypot term contains a part which resolves to the
		 * difference of these two axis lengths in the absence
		 * of rotation.)
		 *
		 * In the calculus, the ratio of the EB and (EA-EC) terms
		 * ends up being the tangent of 2*theta where theta is
		 * the angle that the long axis of the ellipse makes
		 * with the horizontal axis.  Thus, this equation is
		 * calculating the length of the hypotenuse of a triangle
		 * along that axis.
		 */
		double hypot = Math.sqrt(EB*EB + (EA-EC)*(EA-EC));

		/* sqrt omitted, compare to squared limits below. */
		widthsquared = ((EA + EC + hypot)/2.0);
	    }
	    if (bs != defaultStroke) {
		widthsquared *= bs.getLineWidth() * bs.getLineWidth();
	    }
	    if (widthsquared <=
		(aa ? MinPenSizeAASquared : MinPenSizeSquared))
	    {
		if (bs.getDashArray() == null) {
		    strokeState = STROKE_THIN;
		} else {
		    strokeState = STROKE_THINDASHED;
		}
	    } else {
		strokeState = STROKE_WIDE;
	    }
	}
    }

    /*
     * Sets the Stroke in the current graphics state.
     * @param s The Stroke object to be used to stroke a Path in
     * the rendering process.
     * @see BasicStroke
     */
    public void setStroke(Stroke s) {
	if (s == null) {
	    throw new IllegalArgumentException("null Stroke");
	}
        int saveStrokeState = strokeState;
        stroke = s;
        if (s instanceof BasicStroke) {
	    validateBasicStroke((BasicStroke) s);
        } else {
            strokeState = STROKE_CUSTOM;
        }
        if (strokeState != saveStrokeState) {
	    invalidatePipe();
        }
    }

    /**
     * Sets the preferences for the rendering algorithms.
     * Hint categories include controls for rendering quality and
     * overall time/quality trade-off in the rendering process.
     * @param hintKey The key of hint to be set. The strings are
     * defined in the RenderingHints class.  
     * @param hintValue The value indicating preferences for the specified
     * hint category. These strings are defined in the RenderingHints
     * class.  
     * @see RenderingHints
     */
    public void setRenderingHint(Key hintKey, Object hintValue) {
	// If we recognize the key, we must recognize the value
	//     otherwise throw an IllegalArgumentException
	//     and do not change the Hints object
	// If we do not recognize the key, just pass it through
	//     to the Hints object untouched
	if (!hintKey.isCompatibleValue(hintValue)) {
	    throw new IllegalArgumentException
		(hintValue+" is not compatible with "+hintKey);
	}
	if (hintKey instanceof SunHints.Key) {
	    boolean stateChanged;
	    boolean textStateChanged = false;
	    boolean recognized = true;
	    SunHints.Key sunKey = (SunHints.Key) hintKey;
	    int newHint = ((SunHints.Value) hintValue).getIndex();
	    switch (sunKey.getIndex()) {
	    case SunHints.INTKEY_RENDERING:
		stateChanged = (renderHint != newHint);
		if (stateChanged) {
		    renderHint = newHint;
		    if (interpolationHint == -1) {
			interpolationType =
			    (newHint == SunHints.INTVAL_RENDER_QUALITY
			     ? AffineTransformOp.TYPE_BILINEAR
			     : AffineTransformOp.TYPE_NEAREST_NEIGHBOR);
		    }
		}
		break;
	    case SunHints.INTKEY_ANTIALIASING:
		stateChanged = (antialiasHint != newHint);
		antialiasHint = newHint;
		if (stateChanged) {
		    textStateChanged =
			(textAntialiasHint ==
			 SunHints.INTVAL_TEXT_ANTIALIAS_DEFAULT);
		    if (strokeState != STROKE_CUSTOM) {
			validateBasicStroke((BasicStroke) stroke);
		    }
		}
		break;
	    case SunHints.INTKEY_TEXT_ANTIALIASING:
		stateChanged = (textAntialiasHint != newHint);
		textStateChanged = stateChanged;
		textAntialiasHint = newHint;
		break;
	    case SunHints.INTKEY_FRACTIONALMETRICS:
		stateChanged = (fractionalMetricsHint != newHint);
		textStateChanged = stateChanged;
		fractionalMetricsHint = newHint;
		break;
	    case SunHints.INTKEY_INTERPOLATION:
		interpolationHint = newHint;
		switch (newHint) {
		case SunHints.INTVAL_INTERPOLATION_BICUBIC:
		    newHint = AffineTransformOp.TYPE_BICUBIC;
		    break;
		case SunHints.INTVAL_INTERPOLATION_BILINEAR:
		    newHint = AffineTransformOp.TYPE_BILINEAR;
		    break;
		default:
		case SunHints.INTVAL_INTERPOLATION_NEAREST_NEIGHBOR:
		    newHint = AffineTransformOp.TYPE_NEAREST_NEIGHBOR;
		    break;
		}
		stateChanged = (interpolationType != newHint);
		interpolationType = newHint;
		break;
	    case SunHints.INTKEY_STROKE_CONTROL:
		stateChanged = (strokeHint != newHint);
		strokeHint = newHint;
		break;
	    default:
		recognized = false;
		stateChanged = false;
		break;
	    }
	    if (recognized) {
		if (stateChanged) {
		    invalidatePipe();
		    if (textStateChanged) {
			fontMetrics = null;
			this.cachedFRC = null;
			validFontInfo = false;
			this.glyphVectorFontInfo = null;
		    }
		}
		if (hints != null) {
		    hints.put(hintKey, hintValue);
		}
		return;
	    }
	}
	// Nothing we recognize so none of "our state" has changed
	if (hints == null) {
	    hints = makeHints(null);
	}
	hints.put(hintKey, hintValue);
    }


    /**
     * Returns the preferences for the rendering algorithms.
     * @param hintCategory The category of hint to be set. The strings
     * are defined in the RenderingHints class.
     * @return The preferences for rendering algorithms. The strings
     * are defined in the RenderingHints class.
     * @see RenderingHints
     */
    public Object getRenderingHint(Key hintKey) {
	if (hints != null) {
	    return hints.get(hintKey);
	}
	if (!(hintKey instanceof SunHints.Key)) {
	    return null;
	}
	int keyindex = ((SunHints.Key)hintKey).getIndex();
	switch (keyindex) {
	case SunHints.INTKEY_RENDERING:
	    return SunHints.Value.get(SunHints.INTKEY_RENDERING,
				      renderHint);
	case SunHints.INTKEY_ANTIALIASING:
	    return SunHints.Value.get(SunHints.INTKEY_ANTIALIASING,
				      antialiasHint);
	case SunHints.INTKEY_TEXT_ANTIALIASING:
	    return SunHints.Value.get(SunHints.INTKEY_TEXT_ANTIALIASING,
				      textAntialiasHint);
	case SunHints.INTKEY_FRACTIONALMETRICS:
	    return SunHints.Value.get(SunHints.INTKEY_FRACTIONALMETRICS,
				      fractionalMetricsHint);
	case SunHints.INTKEY_INTERPOLATION:
	    switch (interpolationHint) {
	    case SunHints.INTVAL_INTERPOLATION_NEAREST_NEIGHBOR:
		return SunHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR;
	    case SunHints.INTVAL_INTERPOLATION_BILINEAR:
		return SunHints.VALUE_INTERPOLATION_BILINEAR;
	    case SunHints.INTVAL_INTERPOLATION_BICUBIC:
		return SunHints.VALUE_INTERPOLATION_BICUBIC;
	    }
	    return null;
	case SunHints.INTKEY_STROKE_CONTROL:
	    return SunHints.Value.get(SunHints.INTKEY_STROKE_CONTROL,
				      strokeHint);
	}
	return null;
    }

    /**
     * Sets the preferences for the rendering algorithms.
     * Hint categories include controls for rendering quality and
     * overall time/quality trade-off in the rendering process.
     * @param hints The rendering hints to be set
     * @see RenderingHints
     */
    public void setRenderingHints(Map<?,?> hints) {
	this.hints = null;
	renderHint = SunHints.INTVAL_RENDER_DEFAULT;
	antialiasHint = SunHints.INTVAL_ANTIALIAS_OFF;
	textAntialiasHint = SunHints.INTVAL_TEXT_ANTIALIAS_DEFAULT;
	fractionalMetricsHint = SunHints.INTVAL_FRACTIONALMETRICS_OFF;
	interpolationHint = -1;
	interpolationType = AffineTransformOp.TYPE_NEAREST_NEIGHBOR;
	boolean customHintPresent = false;
	Iterator iter = hints.keySet().iterator();
	while (iter.hasNext()) {
	    Object key = iter.next();
	    if (key == SunHints.KEY_RENDERING ||
		key == SunHints.KEY_ANTIALIASING ||
		key == SunHints.KEY_TEXT_ANTIALIASING ||
		key == SunHints.KEY_FRACTIONALMETRICS ||
		key == SunHints.KEY_STROKE_CONTROL ||
		key == SunHints.KEY_INTERPOLATION)
	    {
		setRenderingHint((Key) key, hints.get(key));
	    } else {
		customHintPresent = true;
	    }
	}
	if (customHintPresent) {
	    this.hints = makeHints(hints);
	}
	invalidatePipe();
    }

    /**
     * Adds a number of preferences for the rendering algorithms.
     * Hint categories include controls for rendering quality and
     * overall time/quality trade-off in the rendering process.
     * @param hints The rendering hints to be set
     * @see RenderingHints
     */
    public void addRenderingHints(Map<?,?> hints) {
	boolean customHintPresent = false;
	Iterator iter = hints.keySet().iterator();
	while (iter.hasNext()) {
	    Object key = iter.next();
	    if (key == SunHints.KEY_RENDERING ||
		key == SunHints.KEY_ANTIALIASING ||
		key == SunHints.KEY_TEXT_ANTIALIASING ||
		key == SunHints.KEY_FRACTIONALMETRICS ||
		key == SunHints.KEY_STROKE_CONTROL ||
		key == SunHints.KEY_INTERPOLATION)
	    {
		setRenderingHint((Key) key, hints.get(key));
	    } else {
		customHintPresent = true;
	    }
	}
	if (customHintPresent) {
	    if (this.hints == null) {
		this.hints = makeHints(hints);
	    } else {
		this.hints.putAll(hints);
	    }
	}
    }

    /**
     * Gets the preferences for the rendering algorithms.
     * Hint categories include controls for rendering quality and
     * overall time/quality trade-off in the rendering process.
     * @see RenderingHints
     */
    public RenderingHints getRenderingHints() {
	if (hints == null) {
	    return makeHints(null);
	} else {
	    return (RenderingHints) hints.clone();
	}
    }

    RenderingHints makeHints(Map hints) {
	RenderingHints model = new RenderingHints(hints);
	model.put(SunHints.KEY_RENDERING,
		  SunHints.Value.get(SunHints.INTKEY_RENDERING,
				     renderHint));
	model.put(SunHints.KEY_ANTIALIASING,
		  SunHints.Value.get(SunHints.INTKEY_ANTIALIASING,
				     antialiasHint));
	model.put(SunHints.KEY_TEXT_ANTIALIASING,
		  SunHints.Value.get(SunHints.INTKEY_TEXT_ANTIALIASING,
				     textAntialiasHint));
	model.put(SunHints.KEY_FRACTIONALMETRICS,
		  SunHints.Value.get(SunHints.INTKEY_FRACTIONALMETRICS,
				     fractionalMetricsHint));
	Object value;
	switch (interpolationHint) {
	case SunHints.INTVAL_INTERPOLATION_NEAREST_NEIGHBOR:
	    value = SunHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR;
	    break;
	case SunHints.INTVAL_INTERPOLATION_BILINEAR:
	    value = SunHints.VALUE_INTERPOLATION_BILINEAR;
	    break;
	case SunHints.INTVAL_INTERPOLATION_BICUBIC:
	    value = SunHints.VALUE_INTERPOLATION_BICUBIC;
	    break;
	default:
	    value = null;
	    break;
	}
	if (value != null) {
	    model.put(SunHints.KEY_INTERPOLATION, value);
	}
	model.put(SunHints.KEY_STROKE_CONTROL,
		  SunHints.Value.get(SunHints.INTKEY_STROKE_CONTROL,
				     strokeHint));
	return model;
    }
    
    /**
     * Concatenates the current transform of this Graphics2D with a
     * translation transformation.
     * This is equivalent to calling transform(T), where T is an
     * AffineTransform represented by the following matrix:
     * <pre>
     *		[   1    0    tx  ]
     *		[   0    1    ty  ]
     *		[   0    0    1   ]
     * </pre>
     */
    public void translate(double tx, double ty) {
	transform.translate(tx, ty);
	invalidateTransform();
    }

    /**
     * Concatenates the current transform of this Graphics2D with a
     * rotation transformation.
     * This is equivalent to calling transform(R), where R is an
     * AffineTransform represented by the following matrix:
     * <pre>
     *		[   cos(theta)    -sin(theta)    0   ]
     *		[   sin(theta)     cos(theta)    0   ]
     *		[       0              0         1   ]
     * </pre>
     * Rotating with a positive angle theta rotates points on the positive
     * x axis toward the positive y axis.
     * @param theta The angle of rotation in radians.
     */
    public void rotate(double theta) {
	transform.rotate(theta);
	invalidateTransform();
    }

    /**
     * Concatenates the current transform of this Graphics2D with a
     * translated rotation transformation.
     * This is equivalent to the following sequence of calls:
     * <pre>
     *		translate(x, y);
     *		rotate(theta);
     *		translate(-x, -y);
     * </pre>
     * Rotating with a positive angle theta rotates points on the positive
     * x axis toward the positive y axis.
     * @param theta The angle of rotation in radians.
     * @param x The x coordinate of the origin of the rotation
     * @param y The x coordinate of the origin of the rotation
     */
    public void rotate(double theta, double x, double y) {
	transform.rotate(theta, x, y);
	invalidateTransform();
    }

    /**
     * Concatenates the current transform of this Graphics2D with a
     * scaling transformation.
     * This is equivalent to calling transform(S), where S is an
     * AffineTransform represented by the following matrix:
     * <pre>
     *		[   sx   0    0   ]
     *		[   0    sy   0   ]
     *		[   0    0    1   ]
     * </pre>
     */
    public void scale(double sx, double sy) {
	transform.scale(sx, sy);
	invalidateTransform();
    }

    /**
     * Concatenates the current transform of this Graphics2D with a
     * shearing transformation.
     * This is equivalent to calling transform(SH), where SH is an
     * AffineTransform represented by the following matrix:
     * <pre>
     *		[   1   shx   0   ]
     *		[  shy   1    0   ]
     *		[   0    0    1   ]
     * </pre>
     * @param shx The factor by which coordinates are shifted towards the
     * positive X axis direction according to their Y coordinate
     * @param shy The factor by which coordinates are shifted towards the
     * positive Y axis direction according to their X coordinate
     */
    public void shear(double shx, double shy) {
	transform.shear(shx, shy);
	invalidateTransform();
    }

    /**
     * Composes a Transform object with the transform in this
     * Graphics2D according to the rule last-specified-first-applied.
     * If the currrent transform is Cx, the result of composition
     * with Tx is a new transform Cx'.  Cx' becomes the current
     * transform for this Graphics2D.
     * Transforming a point p by the updated transform Cx' is
     * equivalent to first transforming p by Tx and then transforming
     * the result by the original transform Cx.  In other words,
     * Cx'(p) = Cx(Tx(p)).
     * A copy of the Tx is made, if necessary, so further
     * modifications to Tx do not affect rendering.
     * @param Tx The Transform object to be composed with the current
     * transform.
     * @see #setTransform
     * @see AffineTransform
     */
    public void transform(AffineTransform xform) {
        this.transform.concatenate(xform); 
	invalidateTransform();
    }
    
    /**
     * Translate
     */
    public void translate(int x, int y) {
	transform.translate(x, y);
	if (transformState <= TRANSFORM_INT_TRANSLATE) {
	    transX += x;
	    transY += y;
	    transformState = (((transX | transY) == 0) ?
			      TRANSFORM_ISIDENT : TRANSFORM_INT_TRANSLATE);
	} else {
	    invalidateTransform();
	}
    }

    /**
     * Sets the Transform in the current graphics state.
     * @param Tx The Transform object to be used in the rendering process.
     * @see #transform
     * @see TransformChain
     * @see AffineTransform
     */
    public void setTransform(AffineTransform Tx) {
	if ((constrainX|constrainY) == 0) {
	    transform.setTransform(Tx);
	} else {
	    transform.setToTranslation(constrainX, constrainY);
	    transform.concatenate(Tx);
	}
	invalidateTransform();
    }

    protected void invalidateTransform() {
	int type = transform.getType();
	int origTransformState = transformState;
	if (type == AffineTransform.TYPE_IDENTITY) {
            transformState = TRANSFORM_ISIDENT;
	    transX = transY = 0;
	} else if (type == AffineTransform.TYPE_TRANSLATION) {
	    double dtx = transform.getTranslateX();
	    double dty = transform.getTranslateY();
	    transX = (int) Math.floor(dtx + 0.5);
	    transY = (int) Math.floor(dty + 0.5);
	    if (dtx == transX && dty == transY) {
		transformState = TRANSFORM_INT_TRANSLATE;
	    } else {
		transformState = TRANSFORM_ANY_TRANSLATE;
	    }
	} else if ((type & (AffineTransform.TYPE_FLIP |
			    AffineTransform.TYPE_MASK_ROTATION |
			    AffineTransform.TYPE_GENERAL_TRANSFORM)) == 0)
	{
	    transformState = TRANSFORM_TRANSLATESCALE;
	    transX = transY = 0;
	} else {
	    transformState = TRANSFORM_GENERIC;
	    transX = transY = 0;
	}

	if (transformState >= TRANSFORM_TRANSLATESCALE ||
	    origTransformState >= TRANSFORM_TRANSLATESCALE)
	{
	    /* Its only in this case that the previous or current transform
	     * was more than a translate that font info is invalidated
	     */
	    cachedFRC = null;
	    this.validFontInfo = false;
	    this.fontMetrics = null;
	    this.glyphVectorFontInfo = null;

	    if (transformState != origTransformState) {
		invalidatePipe();
	    }
	}
	if (strokeState != STROKE_CUSTOM) {
	    validateBasicStroke((BasicStroke) stroke);
	}
    }

    /**
     * Returns the current Transform in the Graphics2D state.
     * @see #transform
     * @see #setTransform
     */
    public AffineTransform getTransform() {
	if ((constrainX|constrainY) == 0) {
	    return new AffineTransform(transform);
	}
	AffineTransform tx =
	    AffineTransform.getTranslateInstance(-constrainX, -constrainY);
	tx.concatenate(transform);
        return tx;
    }

    /**
     * Returns the current Transform ignoring the "constrain"
     * rectangle.
     */
    public AffineTransform cloneTransform() {
	return new AffineTransform(transform);
    }

    /**
     * Returns the current Paint in the Graphics2D state.
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     */
    public Paint getPaint() {
        return paint;
    }

    /**
     * Returns the current Composite in the Graphics2D state.
     * @see #setComposite
     */
    public Composite getComposite() {
        return composite;
    }

    public Color getColor() {
        return foregroundColor;
    }

    public void setColor(Color color) {
	if (color == null || color == paint) {
	    return;
	}
        this.paint = foregroundColor = color;
	int rgb;
	this.rgb = rgb = foregroundColor.getRGB();
	pixel = surfaceData.pixelFor(rgb);
	if ((rgb >> 24) == -1) {
	    if (paintState == PAINT_SOLIDCOLOR) {
		return;
	    }
	    paintState = PAINT_SOLIDCOLOR;
	    if (imageComp == CompositeType.SrcOverNoEa) {
		// special case where fillComp depends on opacity of paint
		compositeState = COMP_ISCOPY;
		fillComp = CompositeType.SrcNoEa;
		alphafill = null;
	    }
	} else {
	    if (paintState == PAINT_SINGLECOLOR) {
		return;
	    }
	    paintState = PAINT_SINGLECOLOR;
	    if (imageComp == CompositeType.SrcOverNoEa) {
		// special case where fillComp depends on opacity of paint
		compositeState = COMP_ALPHA;
		fillComp = CompositeType.SrcOverNoEa;
		alphafill = null;
	    }
	}
	invalidatePipe();
    }

    /**
     * Sets the background color in this context used for clearing a region.
     * When Graphics2D is constructed for a component, the backgroung color is
     * inherited from the component. Setting the background color in the
     * Graphics2D context only affects the subsequent clearRect() calls and
     * not the background color of the component. To change the background
     * of the component, use appropriate methods of the component.
     * @param color The background color that should be used in
     * subsequent calls to clearRect().
     * @see getBackground
     * @see Graphics.clearRect()
     */
    public void setBackground(Color color) {
        backgroundColor = color;
    }

    /**
     * Returns the background color used for clearing a region.
     * @see setBackground
     */
    public Color getBackground() {
        return backgroundColor;
    }

    /**
     * Returns the current Stroke in the Graphics2D state.
     * @see setStroke
     */
    public Stroke getStroke() {
        return stroke;
    }
    
    public Rectangle getClipBounds() {
	Rectangle r;
	if (clipState == CLIP_DEVICE) {
	    r = null;
	} else if (transformState <= TRANSFORM_INT_TRANSLATE) {
	    if (usrClip instanceof Rectangle) {
		r = new Rectangle((Rectangle) usrClip);
	    } else {
		r = usrClip.getBounds();
	    }
	    r.translate(-transX, -transY);
	} else {
	    r = getClip().getBounds();
	}
	return r;
    }

    public Rectangle getClipBounds(Rectangle r) {
	if (clipState != CLIP_DEVICE) {
	    if (transformState <= TRANSFORM_INT_TRANSLATE) {
		if (usrClip instanceof Rectangle) {
		    r.setBounds((Rectangle) usrClip);
		} else {
		    r.setBounds(usrClip.getBounds());
		}
		r.translate(-transX, -transY);
	    } else {
		r.setBounds(getClip().getBounds());
	    }
	} else if (r == null) {
	    throw new NullPointerException("null rectangle parameter");
	}
	return r;
    }

    public boolean hitClip(int x, int y, int width, int height) {
	if (width <= 0 || height <= 0) {
	    return false;
	}
	if (transformState > TRANSFORM_INT_TRANSLATE) {
	    // Note: Technically the most accurate test would be to
	    // raster scan the parallelogram of the transformed rectangle
	    // and do a span for span hit test against the clip, but for
	    // speed we approximate the test with a bounding box of the
	    // transformed rectangle.  The cost of rasterizing the
	    // transformed rectangle is probably high enough that it is
	    // not worth doing so to save the caller from having to call
	    // a rendering method where we will end up discovering the
	    // same answer in about the same amount of time anyway.
	    // This logic breaks down if this hit test is being performed
	    // on the bounds of a group of shapes in which case it might
	    // be beneficial to be a little more accurate to avoid lots
	    // of subsequent rendering calls.  In either case, this relaxed
	    // test should not be significantly less accurate than the
	    // optimal test for most transforms and so the conservative
	    // answer should not cause too much extra work.

	    double d[] = {
		x, y,
		x+width, y,
		x, y+height,
		x+width, y+height
	    };
	    transform.transform(d, 0, d, 0, 4);
	    x = (int) Math.floor(Math.min(Math.min(d[0], d[2]),
					  Math.min(d[4], d[6])));
	    y = (int) Math.floor(Math.min(Math.min(d[1], d[3]),
					  Math.min(d[5], d[7])));
	    width = (int) Math.ceil(Math.max(Math.max(d[0], d[2]),
					     Math.max(d[4], d[6])));
	    height = (int) Math.ceil(Math.max(Math.max(d[1], d[3]),
					      Math.max(d[5], d[7])));
	} else {
	    x += transX;
	    y += transY;
	    width += x;
	    height += y;
	}
	if (!getCompClip().intersectsQuickCheckXYXY(x, y, width, height)) {
	    return false;
	}
	// REMIND: We could go one step further here and examine the
	// non-rectangular clip shape more closely if there is one.
	// Since the clip has already been rasterized, the performance
	// penalty of doing the scan is probably still within the bounds
	// of a good tradeoff between speed and quality of the answer.
	return true;
    }

    protected void invalidateClip() {
	clipRegion = null;
	int origClipState = clipState;
	if (usrClip == null) {
	    clipState = CLIP_DEVICE;
	} else if (usrClip instanceof Rectangle2D) {
	    clipState = CLIP_RECTANGULAR;
	} else {
	    clipState = CLIP_SHAPE;
	}
	getCompClip();
	if (origClipState != clipState &&
	    (clipState == CLIP_SHAPE || origClipState == CLIP_SHAPE))
	{
	    invalidatePipe();
	}
    }

    static final int NON_RECTILINEAR_TRANSFORM_MASK =
	(AffineTransform.TYPE_GENERAL_TRANSFORM |
	 AffineTransform.TYPE_GENERAL_ROTATION);

    protected Shape transformShape(Shape s) {
	if (s == null) {
	    return null;
	}
	if (transformState > TRANSFORM_INT_TRANSLATE) {
	    return transformShape(transform, s);
	} else {
	    return transformShape(transX, transY, s);
	}
    }

    public Shape untransformShape(Shape s) {
	if (s == null) {
	    return null;
	}
	if (transformState > TRANSFORM_INT_TRANSLATE) {
	    try {
		return transformShape(transform.createInverse(), s);
	    } catch (NoninvertibleTransformException e) {
		return null;
	    }
	} else {
	    return transformShape(-transX, -transY, s);
	}
    }

    protected static Shape transformShape(int tx, int ty, Shape s) {
	if (s == null) {
	    return null;
	}

	if (s instanceof Rectangle) {
	    Rectangle r = s.getBounds();
	    r.translate(tx, ty);
	    return r;
	}
	if (s instanceof Rectangle2D) {
	    Rectangle2D rect = (Rectangle2D) s;
	    return new Rectangle2D.Double(rect.getX() + tx,
					  rect.getY() + ty,
					  rect.getWidth(),
					  rect.getHeight());
	}

	if (tx == 0 && ty == 0) {
	    return cloneShape(s);
	}

	AffineTransform mat = AffineTransform.getTranslateInstance(tx, ty);
	return mat.createTransformedShape(s);
    }

    protected static Shape transformShape(AffineTransform tx, Shape clip) {
        if (clip == null) {
            return null;
        }

	if (clip instanceof Rectangle2D &&
	    (tx.getType() & NON_RECTILINEAR_TRANSFORM_MASK) == 0)
	{
	    Rectangle2D rect = (Rectangle2D) clip;
	    double matrix[] = new double[4];
	    matrix[0] = rect.getX();
	    matrix[1] = rect.getY();
	    matrix[2] = matrix[0] + rect.getWidth();
	    matrix[3] = matrix[1] + rect.getHeight();
	    tx.transform(matrix, 0, matrix, 0, 2);
	    rect = new Rectangle2D.Float();
	    rect.setFrameFromDiagonal(matrix[0], matrix[1],
				      matrix[2], matrix[3]);
	    return rect;
	}

        if (tx.isIdentity()) {
	    return cloneShape(clip);
        }

	return tx.createTransformedShape(clip);
    }

    public void clipRect(int x, int y, int w, int h) {
	clip(new Rectangle(x, y, w, h));
    }

    public void setClip(int x, int y, int w, int h) {
	setClip(new Rectangle(x, y, w, h));
    }

    public Shape getClip() {
	return untransformShape(usrClip);
    }

    public void setClip(Shape sh) {
	usrClip = transformShape(sh);
	invalidateClip();
    }

    /**
     * Intersects the current clip with the specified Path and sets the
     * current clip to the resulting intersection. The clip is transformed
     * with the current transform in the Graphics2D state before being
     * intersected with the current clip. This method is used to make the
     * current clip smaller. To make the clip larger, use any setClip method.
     * @param p The Path to be intersected with the current clip.
     */
    public void clip(Shape s) {
	s = transformShape(s);
	if (usrClip != null) {
	    s = intersectShapes(usrClip, s, true, true);
	}
	usrClip = s;
	invalidateClip();
    }

    public void setPaintMode() {
        setComposite(AlphaComposite.SrcOver);
    }
    
    public void setXORMode(Color c) {
	if (c == null) {
	    throw new IllegalArgumentException("null XORColor");
	}
        setComposite(new XORComposite(c, surfaceData));
    }

    Blit lastCAblit;
    Composite lastCAcomp;

    public void copyArea(int x, int y, int w, int h, int dx, int dy) {
	if (w <= 0 || h <= 0) {
	    return;
	}
	SurfaceData theData = surfaceData;
	if (theData.copyArea(this, x, y, w, h, dx, dy)) {
	    return;
	}
	if (transformState >= TRANSFORM_TRANSLATESCALE) {
	    throw new InternalError("transformed copyArea not implemented yet");
	}
        if (clipState == CLIP_SHAPE) {
	    throw new InternalError("clipped copyArea not implemented yet");
	}
	// REMIND: This method does not deal with missing data from the
	// source object (i.e. it does not send exposure events...)

	Composite comp = composite;
	if (lastCAcomp != comp) {
	    SurfaceType dsttype = theData.getSurfaceType();
	    CompositeType comptype = imageComp;
	    if (CompositeType.SrcOverNoEa.equals(comptype) &&
		theData.getTransparency() == Transparency.OPAQUE)
	    {
		comptype = CompositeType.SrcNoEa;
	    }
	    lastCAblit = Blit.locate(dsttype, comptype, dsttype);
	    lastCAcomp = comp;
	}

	x += transX;
	y += transY;
	
	Blit ob = lastCAblit;
	if (dy == 0 && dx > 0 && dx < w) {
	    while (w > 0) {
		int partW = Math.min(w, dx);
		w -= partW;
		int sx = x + w;
		ob.Blit(theData, theData, comp, null,
			sx, y, sx+dx, y+dy, partW, h);
	    }
	    return;
	}
	if (dy > 0 && dy < h && dx > -w && dx < w) {
	    while (h > 0) {
		int partH = Math.min(h, dy);
		h -= partH;
		int sy = y + h;
		ob.Blit(theData, theData, comp, null,
			x, sy, x+dx, sy+dy, w, partH);
	    }
	    return;
	}
	ob.Blit(theData, theData, comp, null, x, y, x+dx, y+dy, w, h);
    }

    /*
    public void XcopyArea(int x, int y, int w, int h, int dx, int dy) {
	Rectangle rect = new Rectangle(x, y, w, h);
	rect = transformBounds(rect, transform);
        Point2D    point = new Point2D.Float(dx, dy);
        Point2D    root  = new Point2D.Float(0, 0);
        point = transform.transform(point, point);
        root  = transform.transform(root, root);
        int fdx = (int)(point.getX()-root.getX());
        int fdy = (int)(point.getY()-root.getY());

        Rectangle r = getCompBounds().intersection(rect.getBounds());

        if (r.isEmpty()) {
            return;
        }

        // Begin Rasterizer for Clip Shape
        boolean skipClip = true;
        byte[] clipAlpha = null;

        if (clipState == CLIP_SHAPE) {

	    int box[] = new int[4];

	    clipRegion.getBounds(box);
	    Rectangle devR = new Rectangle(box[0], box[1],
					   box[2] - box[0],
					   box[3] - box[1]);
	    if (!devR.isEmpty()) {
		OutputManager mgr = getOutputManager();
		RegionIterator ri = clipRegion.getIterator();
		while (ri.nextYRange(box)) {
		    int spany = box[1];
		    int spanh = box[3] - spany;
		    while (ri.nextXBand(box)) {
			int spanx = box[0];
			int spanw = box[2] - spanx;
			mgr.copyArea(this, null,
				     spanw, 0,
				     spanx, spany,
				     spanw, spanh,
				     fdx, fdy,
				     null);
		    }
		}
	    }
	    return;
	}
        // End Rasterizer for Clip Shape

        getOutputManager().copyArea(this, null,
				    r.width, 0,
				    r.x, r.y, r.width,
				    r.height, fdx, fdy,
				    null);
    }
    */

    public void drawLine(int x1, int y1, int x2, int y2) {
	try {
	    drawpipe.drawLine(this, x1, y1, x2, y2);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawLine(this, x1, y1, x2, y2);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawRoundRect(int x, int y, int w, int h, int arcW, int arcH) {
	try {
	    drawpipe.drawRoundRect(this, x, y, w, h, arcW, arcH);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawRoundRect(this, x, y, w, h, arcW, arcH);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void fillRoundRect(int x, int y, int w, int h, int arcW, int arcH) {
	try {
	    fillpipe.fillRoundRect(this, x, y, w, h, arcW, arcH);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		fillpipe.fillRoundRect(this, x, y, w, h, arcW, arcH);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawOval(int x, int y, int w, int h) {
	try {
	    drawpipe.drawOval(this, x, y, w, h);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawOval(this, x, y, w, h);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void fillOval(int x, int y, int w, int h) {
	try {
	    fillpipe.fillOval(this, x, y, w, h);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		fillpipe.fillOval(this, x, y, w, h);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawArc(int x, int y, int w, int h,
			int startAngl, int arcAngl) {
	try {
	    drawpipe.drawArc(this, x, y, w, h, startAngl, arcAngl);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawArc(this, x, y, w, h, startAngl, arcAngl);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void fillArc(int x, int y, int w, int h,
			int startAngl, int arcAngl) {
	try {
	    fillpipe.fillArc(this, x, y, w, h, startAngl, arcAngl);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		fillpipe.fillArc(this, x, y, w, h, startAngl, arcAngl);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawPolyline(int xPoints[], int yPoints[], int nPoints) {
	try {
	    drawpipe.drawPolyline(this, xPoints, yPoints, nPoints);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawPolyline(this, xPoints, yPoints, nPoints);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawPolygon(int xPoints[], int yPoints[], int nPoints) {
	try {
	    drawpipe.drawPolygon(this, xPoints, yPoints, nPoints);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawPolygon(this, xPoints, yPoints, nPoints);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void fillPolygon(int xPoints[], int yPoints[], int nPoints) {
	try {
	    fillpipe.fillPolygon(this, xPoints, yPoints, nPoints);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		fillpipe.fillPolygon(this, xPoints, yPoints, nPoints);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawRect (int x, int y, int w, int h) {
	try {
	    drawpipe.drawRect(this, x, y, w, h);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		drawpipe.drawRect(this, x, y, w, h);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }
    
    public void fillRect (int x, int y, int w, int h) {
	try {
	    fillpipe.fillRect(this, x, y, w, h);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		fillpipe.fillRect(this, x, y, w, h);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    private void revalidateAll() {
	try {
	    // REMIND: This locking needs to be done around the
	    // caller of this method so that the pipe stays valid
	    // long enough to call the new primitive.
	    // REMIND: No locking yet in screen SurfaceData objects!
	    // surfaceData.lock();
	    surfaceData = surfaceData.getReplacement();
	    if (surfaceData == null) {
		surfaceData = NullSurfaceData.theInstance;
	    }
	    setDevClip(surfaceData.getBounds());
	    pixel = surfaceData.pixelFor(rgb);
	    if (composite instanceof XORComposite) {
		Color c = ((XORComposite) composite).getXorColor();
		setComposite(new XORComposite(c, surfaceData));
	    }
	    validatePipe();
	} finally {
	    // REMIND: No locking yet in screen SurfaceData objects!
	    // surfaceData.unlock();
	}
    }

    public void clearRect(int x, int y, int w, int h) {
        // REMIND: has some "interesting" consequences if threads are
        // not synchronized
        Composite c = composite;
	Paint p = paint;
        setComposite(AlphaComposite.Src);
        setColor(getBackground());
	validatePipe();
        fillRect(x, y, w, h);
        setPaint(p);
	setComposite(c);
    }

    /**
     * Strokes the outline of a Path using the settings of the current
     * graphics state.  The rendering attributes applied include the
     * clip, transform, paint or color, composite and stroke attributes.
     * @param p The path to be drawn.
     * @see #setStroke
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see #transform
     * @see #setTransform
     * @see #clip
     * @see #setClip
     * @see #setComposite
     */
    public void draw(Shape s) {
	try {
	    shapepipe.draw(this, s);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		shapepipe.draw(this, s);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }


    /**
     * Fills the interior of a Path using the settings of the current
     * graphics state. The rendering attributes applied include the
     * clip, transform, paint or color, and composite.
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see #transform
     * @see #setTransform
     * @see #setComposite
     * @see #clip
     * @see #setClip
     */
    public void fill(Shape s) {
	try {
	    shapepipe.fill(this, s);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		shapepipe.fill(this, s);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    /**
     * Returns true if the given AffineTransform is an integer
     * translation.
     */
    private static boolean isIntegerTranslation(AffineTransform xform) {
        if (xform.isIdentity()) {
            return true;
        }
        if (xform.getType() == AffineTransform.TYPE_TRANSLATION) {
            double tx = xform.getTranslateX();
            double ty = xform.getTranslateY();
            return (tx == (int)tx && ty == (int)ty);
        }
        return false;
    }

    /**
     * Returns the index of the tile corresponding to the supplied position
     * given the tile grid offset and size along the same axis.
     */
    private static int getTileIndex(int p, int tileGridOffset, int tileSize) {
        p -= tileGridOffset;
        if (p < 0) {
            p += 1 - tileSize;		// force round to -infinity (ceiling)
        }
        return p/tileSize;
    }

    /**
     * Returns a rectangle in image coordinates that may be required
     * in order to draw the given image into the given clipping region
     * through a pair of AffineTransforms.  In addition, horizontal and
     * vertical padding factors for antialising and interpolation may
     * be used.
     */
    private static Rectangle getImageRegion(RenderedImage img,
                                            Region compClip,
                                            AffineTransform transform,
                                            AffineTransform xform,
                                            int padX, int padY) {
        Rectangle imageRect =
            new Rectangle(img.getMinX(), img.getMinY(),
                          img.getWidth(), img.getHeight());
        
        Rectangle result = null;
        try {
	    double p[] = new double[8];
	    p[0] = p[2] = compClip.getLoX();
	    p[4] = p[6] = compClip.getHiX();
	    p[1] = p[5] = compClip.getLoY();
	    p[3] = p[7] = compClip.getHiY();

            // Inverse transform the output bounding rect
	    transform.inverseTransform(p, 0, p, 0, 4);
	    xform.inverseTransform(p, 0, p, 0, 4);

            // Determine a bounding box for the inverse transformed region
            double x0,x1,y0,y1;
            x0 = x1 = p[0];
            y0 = y1 = p[1];

            for (int i = 2; i < 8; ) {
                double pt = p[i++];
                if (pt < x0)  {
                    x0 = pt;
                } else if (pt > x1) {
                    x1 = pt;
                }
                pt = p[i++];
                if (pt < y0)  {
                    y0 = pt;
                } else if (pt > y1) { 
                    y1 = pt;
                }
            }

            // This is padding for anti-aliasing and such.  It may
            // be more than is needed.
            int x = (int)x0 - padX;
            int w = (int)(x1 - x0 + 2*padX);
            int y = (int)y0 - padY;
            int h = (int)(y1 - y0 + 2*padY);

            Rectangle clipRect = new Rectangle(x,y,w,h);
            result = clipRect.intersection(imageRect);
        } catch (NoninvertibleTransformException nte) {
            // Worst case bounds are the bounds of the image.
            result = imageRect;
        }

        return result;
    }

    /**
     * Draws an image, applying a transform from image space into user space
     * before drawing.
     * The transformation from user space into device space is done with
     * the current transform in the Graphics2D.
     * The given transformation is applied to the image before the
     * transform attribute in the Graphics2D state is applied.
     * The rendering attributes applied include the clip, transform,
     * and composite attributes. Note that the result is
     * undefined, if the given transform is noninvertible.
     * @param img The image to be drawn. Does nothing if img is null.
     * @param xform The transformation from image space into user space.
     * @see #transform
     * @see #setTransform
     * @see #setComposite
     * @see #clip
     * @see #setClip
     */
    public void drawRenderedImage(RenderedImage img,
                                  AffineTransform xform) {

	if (img == null) {
	    return;
	}

        // BufferedImage case: use a simple drawImage call
        if (img instanceof BufferedImage) {
            BufferedImage bufImg = (BufferedImage)img;
            drawImage(bufImg,xform,null);
            return;
        }

        // transformState tracks the state of transform and
        // transX, transY contain the integer casts of the
        // translation factors
        boolean isIntegerTranslate =
	    (transformState <= TRANSFORM_INT_TRANSLATE) &&
	    isIntegerTranslation(xform);

        // Include padding for interpolation/antialiasing if necessary
        int pad = isIntegerTranslate ? 0 : 3;

        // Determine the region of the image that may contribute to
        // the clipped drawing area
        Rectangle region = getImageRegion(img,
                                          getCompClip(),
                                          transform,
                                          xform,
                                          pad, pad); 
        if (region.width <= 0 || region.height <= 0) {
            return;
        }
        
        // Attempt to optimize integer translation of tiled images.
        // Although theoretically we are O.K. if the concatenation of
        // the user transform and the device transform is an integer
        // translation, we'll play it safe and only optimize the case
        // where both are integer translations.
        if (isIntegerTranslate) {
            // Use optimized code
            // Note that drawTranslatedRenderedImage calls copyImage
            // which takes the user space to device space transform into
            // account, but we need to provide the image space to user space
            // translations.

            drawTranslatedRenderedImage(img, region,
                                        (int) xform.getTranslateX(),
                                        (int) xform.getTranslateY());
            return;
        }

        // General case: cobble the necessary region into a single Raster
        Raster raster = img.getData(region);
        
        // Make a new Raster with the same contents as raster
        // but starting at (0, 0).  This raster is thus in the same
        // coordinate system as the SampleModel of the original raster.
        WritableRaster wRaster =
              Raster.createWritableRaster(raster.getSampleModel(),
                                          raster.getDataBuffer(),
                                          null);

        // If the original raster was in a different coordinate
        // system than its SampleModel, we need to perform an
        // additional translation in order to get the (minX, minY)
        // pixel of raster to be pixel (0, 0) of wRaster.  We also
        // have to have the correct width and height.
        int minX = raster.getMinX();
        int minY = raster.getMinY();
        int width = raster.getWidth();
        int height = raster.getHeight();
        int px = minX - raster.getSampleModelTranslateX();
        int py = minY - raster.getSampleModelTranslateY();
        if (px != 0 || py != 0 || width != wRaster.getWidth() ||
            height != wRaster.getHeight()) {
            wRaster =
                wRaster.createWritableChild(px,
                                            py,
                                            width,
                                            height,
                                            0, 0,
                                            null);
        }

        // Now we have a BufferedImage starting at (0, 0)
        // with the same contents that started at (minX, minY)
        // in raster.  So we must draw the BufferedImage with a
        // translation of (minX, minY).
        AffineTransform transXform = (AffineTransform)xform.clone();
        transXform.translate(minX, minY);
      
        ColorModel cm = img.getColorModel();
        BufferedImage bufImg = new BufferedImage(cm,
                                                 wRaster,
                                                 cm.isAlphaPremultiplied(),
                                                 null);
        drawImage(bufImg, transXform, null);
    }

    /**
     * Intersects <code>destRect</code> with <code>clip</code> and
     * overwrites <code>destRect</code> with the result.
     * Returns false if the intersection was empty, true otherwise.
     */
    private boolean clipTo(Rectangle destRect, Rectangle clip) {
	int x1 = Math.max(destRect.x, clip.x);
	int x2 = Math.min(destRect.x + destRect.width, clip.x + clip.width);
	int y1 = Math.max(destRect.y, clip.y);
	int y2 = Math.min(destRect.y + destRect.height, clip.y + clip.height);
        if (((x2 - x1) < 0) || ((y2 - y1) < 0)) {
            destRect.width = -1; // Set both just to be safe
            destRect.height = -1;
            return false;
        } else {
            destRect.x = x1;
            destRect.y = y1;
            destRect.width = x2 - x1;
            destRect.height = y2 - y1;
            return true;
        }
    }

    /**
     * Draw a portion of a RenderedImage tile-by-tile with a given
     * integer image to user space translation.  The user to
     * device transform must also be an integer translation.
     */
    private void drawTranslatedRenderedImage(RenderedImage img,
                                             Rectangle region,
                                             int i2uTransX,
                                             int i2uTransY) {
        // Cache tile grid info
        int tileGridXOffset = img.getTileGridXOffset();
        int tileGridYOffset = img.getTileGridYOffset();
        int tileWidth = img.getTileWidth();
        int tileHeight = img.getTileHeight();
        
        // Determine the tile index extrema in each direction
        int minTileX =
            getTileIndex(region.x, tileGridXOffset, tileWidth);
        int minTileY =
            getTileIndex(region.y, tileGridYOffset, tileHeight);
        int maxTileX =
            getTileIndex(region.x + region.width - 1,
                         tileGridXOffset, tileWidth);
        int maxTileY =
            getTileIndex(region.y + region.height - 1,
                         tileGridYOffset, tileHeight);

        // Create a single ColorModel to use for all BufferedImages
        ColorModel colorModel = img.getColorModel();

        // Reuse the same Rectangle for each iteration
        Rectangle tileRect = new Rectangle();

        for (int ty = minTileY; ty <= maxTileY; ty++) {
            for (int tx = minTileX; tx <= maxTileX; tx++) {
                // Get the current tile.
                Raster raster = img.getTile(tx, ty);

                // Fill in tileRect with the tile bounds
                tileRect.x = tx*tileWidth + tileGridXOffset;
                tileRect.y = ty*tileHeight + tileGridYOffset;
                tileRect.width = tileWidth;
                tileRect.height = tileHeight;

                // Clip the tile against the image bounds and
                // backwards mapped clip region
                // The result can't be empty
                clipTo(tileRect, region);

                // Create a WritableRaster containing the tile
                WritableRaster wRaster = null;
                if (raster instanceof WritableRaster) {
                    wRaster = (WritableRaster)raster;
                } else {
                    // Create a WritableRaster in the same coordinate system
                    // as the original raster.
                    wRaster =
                        Raster.createWritableRaster(raster.getSampleModel(),
                                                    raster.getDataBuffer(),
                                                    null);
                }

                // Translate wRaster to start at (0, 0) and to contain
                // only the relevent portion of the tile
                wRaster = wRaster.createWritableChild(tileRect.x, tileRect.y,
                                                      tileRect.width,
                                                      tileRect.height,
                                                      0, 0,
                                                      null);

                // Wrap wRaster in a BufferedImage
                BufferedImage bufImg =
                    new BufferedImage(colorModel,
                                      wRaster,
                                      colorModel.isAlphaPremultiplied(),
                                      null);
                // Now we have a BufferedImage starting at (0, 0) that
                // represents data from a Raster starting at
                // (tileRect.x, tileRect.y).  Additionally, it needs
                // to be translated by (i2uTransX, i2uTransY).  We call
                // copyImage to draw just the region of interest
                // without needing to create a child image.
		copyImage(bufImg, tileRect.x + i2uTransX,
                          tileRect.y + i2uTransY, 0, 0, tileRect.width,
                          tileRect.height, null, null);
            }
        }
    }

    public void drawRenderableImage(RenderableImage img,
                                    AffineTransform xform) {

	if (img == null) {
	    return;
	}

        AffineTransform pipeTransform = transform;
        AffineTransform concatTransform = new AffineTransform(xform);
        concatTransform.concatenate(pipeTransform);
        AffineTransform reverseTransform;
 
        RenderContext rc = new RenderContext(concatTransform);

        try {
            reverseTransform = pipeTransform.createInverse();
        } catch (NoninvertibleTransformException nte) {
            rc = new RenderContext(pipeTransform);
            reverseTransform = new AffineTransform();
        }

        RenderedImage rendering = img.createRendering(rc);
        drawRenderedImage(rendering,reverseTransform);
    }


    
    /*
     * Transform the bounding box of the BufferedImage
     */
    protected Rectangle transformBounds(Rectangle rect,
                                        AffineTransform tx) {
        if (tx.isIdentity()) {
            return rect;
        }

        Shape s = transformShape(tx, rect);
        return s.getBounds();
    }

    // text rendering methods
    public void drawString(String str, int x, int y) {
        if (str == null) {
	    throw new NullPointerException("String is null");
	}

        try {
	    textpipe.drawString(this, str, x, y);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		textpipe.drawString(this, str, x, y);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    public void drawString(String str, float x, float y) {
        if (str == null) {
	    throw new NullPointerException("String is null");
	}

        try {
            textpipe.drawString(this, str, x, y);
	} catch (InvalidPipeException e) {
            revalidateAll();
            try {
                textpipe.drawString(this, str, x, y);
            } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.	Fail for now and
		// try again next time around.
            }
        }
    }

    public void drawString(AttributedCharacterIterator iterator,
			   int x, int y) {
        if (iterator == null) {
	    throw new NullPointerException("AttributedCharacterIterator is null");
	}
        TextLayout tl = new TextLayout(iterator, getFontRenderContext());
        tl.draw(this, (float) x, (float) y);
    }

    public void drawString(AttributedCharacterIterator iterator,
			   float x, float y) {
        if (iterator == null) {
	    throw new NullPointerException("AttributedCharacterIterator is null");
	}
        TextLayout tl = new TextLayout(iterator, getFontRenderContext());
        tl.draw(this, x, y);
    }

    public void drawGlyphVector(GlyphVector gv, float x, float y)
    {
        if (gv == null) {
	    throw new NullPointerException("GlyphVector is null");
	}
   
        try {
            textpipe.drawGlyphVector(this, gv, x, y);
        } catch (InvalidPipeException e) {
            revalidateAll();
	    try {
		textpipe.drawGlyphVector(this, gv, x, y);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
        }
    }

    public void drawChars(char data[], int offset, int length, int x, int y) {
        if (data == null) {
	    throw new NullPointerException("char data is null");
	}
	if (length < 0 || offset + length > data.length) {
	    throw new ArrayIndexOutOfBoundsException("bad offset/length");
	}
        try {
            textpipe.drawChars(this, data, offset, length, x, y);
	} catch (InvalidPipeException e) {
            revalidateAll();
            try {
                textpipe.drawChars(this, data, offset, length, x, y);
            } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.	Fail for now and
		// try again next time around.
            }
	}
    }

    public void drawBytes(byte data[], int offset, int length, int x, int y) {
        if (data == null) {
	    throw new NullPointerException("byte data is null");
	}
        /* Byte data is interpreted as 8-bit ASCII. Re-use drawChars loops */
	char chData[] = new char[length];
	for (int i = length; i-- > 0; ) {
            chData[i] = (char)(data[i+offset] & 0xff);
	}
        try {
            textpipe.drawChars(this, chData, 0, length, x, y);
	} catch (InvalidPipeException e) {
            revalidateAll();
            try {
                textpipe.drawChars(this, chData, 0, length, x, y);
            } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.	Fail for now and
		// try again next time around.
            }
	}
    }
// end of text rendering methods

    /**
     * Draws an image scaled to x,y,w,h in nonblocking mode with a
     * callback object.
     */
    public boolean drawImage(Image img, int x, int y, int width, int height,
			     ImageObserver observer) {
	return drawImage(img, x, y, width, height, null, observer);
    }

    /**
     * Not part of the advertised API but a useful utility method
     * to call internally.  This is for the case where we are
     * drawing to/from given coordinates using a given width/height,
     * but we guarantee that the weidth/height of the src and dest
     * areas are equal (no scale needed).
     */
    public boolean copyImage(Image img, int dx, int dy, int sx, int sy, 
			     int width, int height, Color bgcolor,
			     ImageObserver observer) {
	try {
	    return imagepipe.copyImage(this, img, dx, dy, sx, sy, 
				       width, height, bgcolor, observer);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		return imagepipe.copyImage(this, img, dx, dy, sx, sy, 
					   width, height, bgcolor, observer);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
		return false;
	    }
	}
    }
        
    /**
     * Draws an image scaled to x,y,w,h in nonblocking mode with a
     * solid background color and a callback object.
     */
    public boolean drawImage(Image img, int x, int y, int width, int height,
 			     Color bg, ImageObserver observer) {

	if (img == null) {
	    return true;
	}

	if ((width == 0) || (height == 0)) {
	    return true;
	}
	if (width == img.getWidth(null) && height == img.getHeight(null)) {
	    return copyImage(img, x, y, 0, 0, width, height, bg, observer);
	}

	try {
	    return imagepipe.scaleImage(this, img, x, y, width, height, 
					bg, observer);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		return imagepipe.scaleImage(this, img, x, y, width, height, 
					    bg, observer);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
		return false;
	    }
	}
    }

    /**
     * Draws an image at x,y in nonblocking mode.
     */
    public boolean drawImage(Image img, int x, int y, ImageObserver observer) {
	return drawImage(img, x, y, null, observer);
    }

    /**
     * Draws an image at x,y in nonblocking mode with a solid background
     * color and a callback object.
     */
    public boolean drawImage(Image img, int x, int y, Color bg,
			     ImageObserver observer) {

	if (img == null) {
	    return true;
	}

	try {
	    return imagepipe.copyImage(this, img, x, y, bg, observer);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		return imagepipe.copyImage(this, img, x, y, bg, observer);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
		return false;
	    }
	}
    }

    /**
     * Draws a subrectangle of an image scaled to a destination rectangle
     * in nonblocking mode with a callback object.
     */
    public boolean drawImage(Image img,
			     int dx1, int dy1, int dx2, int dy2,
			     int sx1, int sy1, int sx2, int sy2,
			     ImageObserver observer) {
	return drawImage(img, dx1, dy1, dx2, dy2, sx1, sy1, sx2, sy2, null, 
			 observer);
    }

    /**
     * Draws a subrectangle of an image scaled to a destination rectangle in
     * nonblocking mode with a solid background color and a callback object.
     */
    public boolean drawImage(Image img,
			     int dx1, int dy1, int dx2, int dy2,
			     int sx1, int sy1, int sx2, int sy2,
			     Color bgcolor, ImageObserver observer) {

	if (img == null) {
	    return true;
	}

	if (dx1 == dx2 || dy1 == dy2 ||
	    sx1 == sx2 || sy1 == sy2) 
	{
	    return true;
	}

	if (((sx2 - sx1) == (dx2 - dx1)) && 
	    ((sy2 - sy1) == (dy2 - dy1)))
	{
	    // Not a scale - forward it to a copy routine
	    int srcX, srcY, dstX, dstY, width, height;
	    if (sx2 > sx1) {
		width = sx2 - sx1;
		srcX = sx1;
		dstX = dx1;
	    } else {
		width = sx1 - sx2;
		srcX = sx2;
		dstX = dx2;
	    }
	    if (sy2 > sy1) {
		height = sy2-sy1;
		srcY = sy1;
		dstY = dy1;
	    } else {
		height = sy1-sy2;
		srcY = sy2;
		dstY = dy2;
	    }
	    return copyImage(img, dstX, dstY, srcX, srcY, 
			     width, height, bgcolor, observer);
	}

	try {
	    return imagepipe.scaleImage(this, img, dx1, dy1, dx2, dy2,
					  sx1, sy1, sx2, sy2, bgcolor, 
					  observer);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		return imagepipe.scaleImage(this, img, dx1, dy1, dx2, dy2,
					      sx1, sy1, sx2, sy2, bgcolor, 
					      observer);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
		return false;
	    }
	}
    }

    /**
     * Draw an image, applying a transform from image space into user space
     * before drawing.
     * The transformation from user space into device space is done with
     * the current transform in the Graphics2D.
     * The given transformation is applied to the image before the
     * transform attribute in the Graphics2D state is applied.
     * The rendering attributes applied include the clip, transform,
     * paint or color and composite attributes. Note that the result is
     * undefined, if the given transform is non-invertible.
     * @param img The image to be drawn.
     * @param xform The transformation from image space into user space.
     * @param observer The image observer to be notified on the image producing
     * progress.
     * @see #transform
     * @see #setComposite
     * @see #setClip
     */
    public boolean drawImage(Image img,
                             AffineTransform xform,
                             ImageObserver observer) {

	if (img == null) {
	    return true;
	}

        if (xform == null || xform.isIdentity()) {
	    return drawImage(img, 0, 0, null, observer); 
	}

	try {
	    return imagepipe.transformImage(this, img, xform, observer);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		return imagepipe.transformImage(this, img, xform, observer);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
		return false;
	    }
	}
    }
    
    public void drawImage(BufferedImage bImg,
                          BufferedImageOp op,
                          int x,
                          int y)  {

	if (bImg == null) {
	    return;
	}

	try {
	    imagepipe.transformImage(this, bImg, op, x, y);
	} catch (InvalidPipeException e) {
	    revalidateAll();
	    try {
		imagepipe.transformImage(this, bImg, op, x, y);
	    } catch (InvalidPipeException e2) {
		// Still catching the exception; we are not yet ready to
		// validate the surfaceData correctly.  Fail for now and 
		// try again next time around.
	    }
	}
    }

    /**
    * Get the rendering context of the font
    * within this Graphics2D context.
    */
    public FontRenderContext getFontRenderContext() {
	if (cachedFRC == null) {
	    int aahint =
		(textAntialiasHint == SunHints.INTVAL_TEXT_ANTIALIAS_DEFAULT
		 ? antialiasHint : textAntialiasHint);
	    cachedFRC = new FontRenderContext
	    ((transformState < TRANSFORM_TRANSLATESCALE) ? null : transform,
	     (aahint == SunHints.INTVAL_ANTIALIAS_ON),
	     (fractionalMetricsHint == SunHints.INTVAL_FRACTIONALMETRICS_ON));
	}
	return cachedFRC;
    }
    private FontRenderContext cachedFRC;

    public void dispose() {
	surfaceData = NullSurfaceData.theInstance;
	invalidatePipe();
    }

    public void finalize() {
    }
}
