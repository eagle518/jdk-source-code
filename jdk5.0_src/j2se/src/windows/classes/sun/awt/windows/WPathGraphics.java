/*
 * @(#)WPathGraphics.java	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Paint;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.Transparency;

import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;

import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.PathIterator;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Line2D;

import java.awt.image.BufferedImage;
import sun.awt.image.ByteComponentRaster;

import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;

import sun.awt.FontConfiguration;
import sun.awt.PlatformFont;

import sun.font.CompositeFont;
import sun.font.Font2D;
import sun.font.FontManager;

import sun.print.PathGraphics;
import sun.print.ProxyGraphics2D;
import sun.java2d.SunGraphicsEnvironment;

class WPathGraphics extends PathGraphics {

    /**
     * For a drawing application the initial user space
     * resolution is 72dpi.
     */
    private static final int DEFAULT_USER_RES = 72;

    private static final float MIN_DEVICE_LINEWIDTH = 1.2f;
    private static final float MAX_THINLINE_INCHES = 0.014f;

    private Font lastFont;
    private Font lastDeviceSizeFont;
    private int lastAngle;
    private float lastScaledFontSize;
    private float lastAverageWidthScale;

    WPathGraphics(Graphics2D graphics, PrinterJob printerJob,
                  Printable painter, PageFormat pageFormat, int pageIndex,
                  boolean canRedraw) {
	super(graphics, printerJob, painter, pageFormat, pageIndex, canRedraw);
    }

    /**
     * Creates a new <code>Graphics</code> object that is 
     * a copy of this <code>Graphics</code> object.
     * @return     a new graphics context that is a copy of 
     *                       this graphics context.
     * @since      JDK1.0
     */
    public Graphics create() {

	return new WPathGraphics((Graphics2D) getDelegate().create(),
				 getPrinterJob(),
                                 getPrintable(),
                                 getPageFormat(),
                                 getPageIndex(),
                                 canDoRedraws());
    }

    /**
     * Strokes the outline of a Shape using the settings of the current
     * graphics state.  The rendering attributes applied include the
     * clip, transform, paint or color, composite and stroke attributes.
     * @param s The shape to be drawn.
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

	Stroke stroke = getStroke();

	/* If the line being drawn is thinner than can be
	 * rendered, then change the line width, stroke
	 * the shape, and then set the line width back.
	 * We can only do this for BasicStroke's.
	 */
	if (stroke instanceof BasicStroke) {
	    BasicStroke lineStroke;
	    BasicStroke minLineStroke = null;
	    float deviceLineWidth;
	    float lineWidth;
	    AffineTransform deviceTransform;
	    Point2D.Float penSize;

	    /* Get the requested line width in user space.
	     */
	    lineStroke = (BasicStroke) stroke;
	    lineWidth = lineStroke.getLineWidth();
	    penSize = new Point2D.Float(lineWidth, lineWidth);

	    /* Compute the line width in device coordinates.
	     * Work on a point in case there is asymetric scaling
	     * between user and device space.
	     * Take the absolute value in case there is negative
	     * scaling in effect.
	     */
	    deviceTransform = getTransform();
	    deviceTransform.deltaTransform(penSize, penSize);
	    deviceLineWidth = Math.min(Math.abs(penSize.x),
				       Math.abs(penSize.y));

	    /* If the requested line is too thin then map our
	     * minimum line width back to user space and set
	     * a new BasicStroke.
	     */
	    if (deviceLineWidth < MIN_DEVICE_LINEWIDTH) {

		Point2D.Float minPenSize = new Point2D.Float(
						MIN_DEVICE_LINEWIDTH,
						MIN_DEVICE_LINEWIDTH);

		try {
		    AffineTransform inverse;
		    float minLineWidth;

		    /* Convert the minimum line width from device
		     * space to user space.
		     */
		    inverse = deviceTransform.createInverse();
		    inverse.deltaTransform(minPenSize, minPenSize);

		    minLineWidth = Math.max(Math.abs(minPenSize.x),
					    Math.abs(minPenSize.y));

		    /* Use all of the parameters from the current
		     * stroke but change the line width to our
		     * calculated minimum.
		     */
		    minLineStroke = new BasicStroke(minLineWidth,
						    lineStroke.getEndCap(),
						    lineStroke.getLineJoin(),
						    lineStroke.getMiterLimit(),
						    lineStroke.getDashArray(),
						    lineStroke.getDashPhase());
		    setStroke(minLineStroke);

    		} catch (NoninvertibleTransformException e) {
		    /* If we can't invert the matrix there is something
		     * very wrong so don't worry about the minor matter
		     * of a minimum line width.
		     */
		}
	    }

	    super.draw(s);

	    /* If we changed the stroke, put back the old
	     * stroke in order to maintain a minimum line
	     * width.
	     */
	    if (minLineStroke != null) {
		setStroke(lineStroke);
	    }

	/* The stroke in effect was not a BasicStroke so we
	 * will not try to enforce a minimum line width.
	 */
	} else {
	    super.draw(s);
	}
    }

    /**
     * Draws the text given by the specified string, using this
     * graphics context's current font and color. The baseline of the
     * first character is at position (<i>x</i>,&nbsp;<i>y</i>) in this
     * graphics context's coordinate system.
     * @param       str      the string to be drawn.
     * @param       x        the <i>x</i> coordinate.
     * @param       y        the <i>y</i> coordinate.
     * @see         java.awt.Graphics#drawBytes
     * @see         java.awt.Graphics#drawChars
     * @since       JDK1.0
     */
    public void drawString(String str, int x, int y) {
        drawString(str, (float) x, (float) y);
    }

    /* A return value of 0 would mean font not available to GDI, or the
     * it can't be used for this string.
     * A return of 1 means it is suitable, including for composites.
     * We check that the transform in effect is doable with GDI, and that
     * this is a composite font AWT can handle, or a physical font GDI
     * can handle directly. Its possible that some strings may ultimately
     * fail the more stringent tests in drawString but this is rare and
     * also that method will always succeed, as if the font isn't available
     * it will use outlines via a superclass call. Also it is only called for
     * the default render context (as canDrawStringToWidth() will return
     * false. That is why it ignores the frc and width arguments.
     */
    protected int platformFontCount(Font font, String str) {

        AffineTransform deviceTransform = getTransform();
	AffineTransform fontTransform = new AffineTransform(deviceTransform);
	fontTransform.concatenate(getFont().getTransform());
        int transformType = fontTransform.getType();

	/* Test if GDI can handle the transform */
	boolean directToGDI = ((transformType !=
			       AffineTransform.TYPE_GENERAL_TRANSFORM)
			       && ((transformType & AffineTransform.TYPE_FLIP)
				   == 0));

	if (!directToGDI) {
	    return 0;
	}

	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

	Font2D font2D = FontManager.getFont2D(font);
	if (font2D instanceof CompositeFont) {
	    if (!((CompositeFont)font2D).isStdComposite() ||
		FontManager.usingAlternateCompositeFonts()) {
		return 0;
	    }
	    return (wPrinterJob.setLogicalFont(font, 0, 1f)) ? 1 : 0;
	}

	return (wPrinterJob.setFont(font, 0, 1f)) ? 1 : 0;
  
    }

    /**
     * Renders the text specified by the specified <code>String</code>,
     * using the current <code>Font</code> and <code>Paint</code> attributes
     * in the <code>Graphics2D</code> context.
     * The baseline of the first character is at position
     * (<i>x</i>,&nbsp;<i>y</i>) in the User Space.
     * The rendering attributes applied include the <code>Clip</code>,
     * <code>Transform</code>, <code>Paint</code>, <code>Font</code> and
     * <code>Composite</code> attributes. For characters in script systems
     * such as Hebrew and Arabic, the glyphs can be rendered from right to
     * left, in which case the coordinate supplied is the location of the
     * leftmost character on the baseline.
     * @param s the <code>String</code> to be rendered
     * @param x,&nbsp;y the coordinates where the <code>String</code>
     * should be rendered
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see java.awt.Graphics#setFont
     * @see #setTransform
     * @see #setComposite
     * @see #setClip
     */
     public void drawString(String str, float x, float y,
			    Font font, FontRenderContext frc, float w) {

        if (str.length() == 0) {
            return;
        }
    	Font oldFont = getFont();
	if (!oldFont.equals(font)) {
	    setFont(font);
	} else {
	    oldFont = null;
	}

        boolean drawnWithGDI = false;

        AffineTransform deviceTransform = getTransform();
	AffineTransform fontTransform = new AffineTransform(deviceTransform);
	fontTransform.concatenate(getFont().getTransform());
        int transformType = fontTransform.getType();

	/* Use GDI for the text if the graphics transform is something
         * for which we can obtain a suitable GDI font.
         * A flip or shearing transform on the graphics or a transform
         * on the font force us to decompose the text into a shape.
         */
	boolean directToGDI = ((transformType !=
			       AffineTransform.TYPE_GENERAL_TRANSFORM)
			       && ((transformType & AffineTransform.TYPE_FLIP)
				   == 0));

	boolean shapingNeeded = stringNeedsShaping(str);

        if (!WPrinterJob.shapeTextProp && directToGDI && !shapingNeeded) {
            /* Compute the starting position of the string in
             * device space.
             */
            Point2D.Float pos = new Point2D.Float(x, y);
            deviceTransform.transform(pos, pos);

            /* Get the font size in device coordinates.
             * Because this code only supports uniformly scaled
             * device transforms, the fontSize must be equal in the
             * x and y directions.
             */
            Font currentFont = getFont();
            float fontSize = currentFont.getSize2D();

            Point2D.Double pty = new Point2D.Double(0.0, 1.0);
            fontTransform.deltaTransform(pty, pty);
            double scaleFactorY = Math.sqrt(pty.x*pty.x+pty.y*pty.y);
            float scaledFontSizeY = (float)(fontSize * scaleFactorY);

            Point2D.Double pt = new Point2D.Double(1.0, 0.0);
            fontTransform.deltaTransform(pt, pt);
            double scaleFactorX = Math.sqrt(pt.x*pt.x+pt.y*pt.y);
            float scaledFontSizeX = (float)(fontSize * scaleFactorX);

	    float awScale =(float)(scaleFactorX/scaleFactorY);
	    /* don't let rounding errors be interpreted as non-uniform scale */
	    if (awScale > 0.999f && awScale < 1.001f) {
		awScale = 1.0f;
	    }
            
            /* Get the rotation in 1/10'ths degree (as needed by Windows)
             * so that GDI can draw the text rotated.
             * This calculation is only valid for a uniform scale, no shearing.
             */
            double angle = Math.toDegrees(Math.atan2(pt.y, pt.x));
            if (angle < 0.0) {
                angle+= 360.0;
            }
            /* Windows specifies the rotation anti-clockwise from the x-axis
             * of the device, 2D specifies +ve rotation towards the y-axis
             * Since the 2D y-axis runs from top-to-bottom, windows angle of
             * rotation here is opposite than 2D's, so the rotation needed
             * needs to be recalculated in the opposite direction.
             */
            if (angle != 0.0) {
               angle = 360.0 - angle;
            }
            int iangle = (int)Math.round(angle * 10.0);
       
            /* If the last font used is identical to the current font
             * then we re-use the previous scaled Java font. This is
             * not just a benefit for Java object re-use, it allows re-use
             * of GDI fonts in the font peer of logical fonts and
             * printer drivers will not need to keep setting the font
             */
            Font deviceSizeFont;
            if ((currentFont != null) && (lastFont != null) && 
                (lastDeviceSizeFont != null) &&
                (scaledFontSizeY == lastScaledFontSize) &&
		(awScale == lastAverageWidthScale) &&
                currentFont.equals(lastFont) && (iangle == lastAngle)) {
                deviceSizeFont = lastDeviceSizeFont;
            } else {
                deviceSizeFont = currentFont.deriveFont(scaledFontSizeY);
                lastAngle = iangle;
                lastScaledFontSize = scaledFontSizeY;
		lastAverageWidthScale = awScale;
                lastDeviceSizeFont = deviceSizeFont;
                lastFont = currentFont;
            }

            /*
             * If there is a mapping from the java font to the GDI
             * font then we can draw the text with GDI. If there is
             * no such mapping then setFont will return false and
             * we'll decompose the text into a Shape.
             *
             */
 	    Font2D font2D = FontManager.getFont2D(deviceSizeFont);
 	    boolean isNonStdComposite =
		(font2D instanceof CompositeFont) &&
		!((CompositeFont)font2D).isStdComposite();
            WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();
	    Font logicalFont = null;
            boolean gotLogicalFont = false;
            boolean gotPhysicalFont =
		wPrinterJob.setFont(deviceSizeFont, iangle, awScale);
	    /* Physical Font may in fact be a Swing composite (see
	     * FontManager.getCompositeFontUIResource()) which
	     * superficially appears to be a physical font but is backed
	     * by a composite. We should detect this case and if it is
	     * check to see if the font can actually display all characters
	     * in the string. If not we need to fall back to 2D outlines.
	     * We don't do this automatically because its non-optimal and
	     * probably rarely needed. Also we don't want to fall
	     * back to outlines for fonts which aren't backed by a
	     * composite as the missing glyph may actually be what is
	     * wanted. Also note that we explicitly create new Font(..)
	     * to perform the canDisplay as otherwise canDisplay will
	     * be returning true because the composite has the glyphs
	     * and we really want to know about the physical font.
	     * background: recall that Swing needs this because windows
	     * will automatically add a suitable font for your locale in
	     * addition to whatever your deskeop and witout this extra
	     * font Swing may not have the glyphs for the user's language.
	     * So the question here is would this windows behaviour work
	     * for printing too - that might make some of this checking
	     * unnecessary except that we are unlikely to have picked the
	     * same fallback as windows did.
	     */
	    if (gotPhysicalFont && isNonStdComposite) {
		Font tmpFont = new Font(deviceSizeFont.getFamily(),
				       deviceSizeFont.getStyle(), 12);
		gotPhysicalFont = (tmpFont.canDisplayUpTo(str) == -1);
	    }
            if (!gotPhysicalFont && 
		SunGraphicsEnvironment.isLogicalFont(deviceSizeFont)) {
		/* If (eg) locale-specific fonts have been selected, then the
		 * logical fonts used by AWT may not be the same as those
		 * used by 2D, so we cannot use the AWT logical font during
		 * printing.
		 * So if the boolean method call on FontManager returns true
		 * then we need to use outlines which is inefficient.
		 * We need a better solution here. We should be able
		 * to decompose the String into physical fonts
		 */
                if (!sun.font.FontManager.usingAlternateCompositeFonts() &&
		    !isNonStdComposite) {
		    /* AWT can't handle a face name, just a family name.
		     * So if the font is a face name need to build a
		     * new font.
		     */
		    logicalFont = deviceSizeFont;
		    if (FontConfiguration.
			isLogicalFontFaceName(logicalFont.getFontName())) {
			int realStyle =
			    font2D.getStyle() | logicalFont.getStyle();
			logicalFont = new Font(deviceSizeFont.getFamily(),
					       realStyle,
					       deviceSizeFont.getSize());
		    }
		    gotLogicalFont = 
			wPrinterJob.setLogicalFont(logicalFont,
						   iangle, awScale);
		}
		if (gotLogicalFont) {
		    try {
		      /* check all chars in string can be converted */
			if (((PlatformFont)logicalFont.getPeer()).
			    makeMultiCharsetString(str, false) == null) 
			  {
			    gotLogicalFont = false;
			}	
		    } catch (Exception e) {
			gotLogicalFont = false;
		    }
		}
            }

            if (gotPhysicalFont || gotLogicalFont) {

                /* Set the text color.
                 * We should not be in this shape printing path
                 * if the application is drawing with non-solid
                 * colors. We should be in the raster path. Because
                 * we are here in the shape path, the cast of the
                 * paint to a Color should be fine.
                 */
                try {
                    wPrinterJob.setTextColor( (Color) getPaint());
                } catch (ClassCastException e) {
		    if (oldFont != null) {
			setFont(oldFont);
		    }
                    throw new IllegalArgumentException(
                                                "Expected a Color instance");
                }

		if (getClip() != null) {
		    deviceClip(getClip().getPathIterator(deviceTransform));
		}

                /* Let GDI draw the text by positioning each glyph
                 * as calculated by windows.
                 * REMIND: as per eval of bug 4271596, we can't use T2K
                 * to calculate the glyph advances
                 */
                wPrinterJob.textOut(str, pos.x, pos.y,
                                    (gotLogicalFont ? logicalFont : null));
                drawnWithGDI = true;
            }
        }

        /* The text could not be converted directly to GDI text
         * calls so decompose the text into a shape.
         */
        if (drawnWithGDI == false) {
	    if (oldFont != null) {
		setFont(oldFont);
		oldFont = null;
	    }
            super.drawString(str, x, y, font, frc, w);
        }

	if (oldFont != null) {
	    setFont(oldFont);
	}
    }

    /* GDI doesn't handle shaping or BIDI consistently with on-screen cases
     * and TextLayout, so we will skip GDI text for Arabic & Hebrew.
     * Results should then be correct for those locales.
     */

    private boolean stringNeedsShaping(String s) {
        boolean shapingNeeded = false;
       
        char[] chars = s.toCharArray();
        char c;
 
        for (int i=0; i<chars.length;i++) {
            c = chars[i];

            if ((c & 0xfe00) == 0) {
                continue; // if roman assume no shaping, BIDI
            }
            if ((c >= 0x0590) && (c <= 0x05ff)) { // Hebrew
                shapingNeeded = true;
                break;
            }
            if ((c >= 0x0600) && (c <= 0x06ff)) { // Arabic
                shapingNeeded = true;
                break;
            }
            if ((c >= 0x202a) && (c <= 0x202e)) { // directional control
                shapingNeeded = true;
                break;
            }
            if ((c >= 0x206a) && (c <= 0x206f)) { // directional control
                shapingNeeded = true;
                break;
            }
          
        } 

        return shapingNeeded;
    }

    /**
     * The various <code>drawImage()</code> methods for
     * <code>WPathGraphics</code> are all decomposed
     * into an invocation of <code>drawImageToPlatform</code>.
     * The portion of the passed in image defined by
     * <code>srcX, srcY, srcWidth, and srcHeight</code>
     * is transformed by the supplied AffineTransform and
     * drawn using GDI to the printer context.
     * 
     * @param	img	The image to be drawn.
     * @param	xform	Used to tranform the image before drawing.
     *			This can be null.
     * @param	bgcolor	This color is drawn where the image has transparent
     *			pixels. If this parameter is null then the
     *			pixels already in the destination should show
     *			through.
     * @param	srcX	With srcY this defines the upper-left corner
     *			of the portion of the image to be drawn.
     *
     * @param	srcY	With srcX this defines the upper-left corner
     *			of the portion of the image to be drawn.
     * @param	srcWidth    The width of the portion of the image to
     *			    be drawn.
     * @param	srcHeight   The height of the portion of the image to
     *			    be drawn.
     * @param   handlingTransparency if being recursively called to
     *                    print opaque region of transparent image
     */
    protected boolean drawImageToPlatform(Image image, AffineTransform xform,
					  Color bgcolor,
					  int srcX, int srcY,
					  int srcWidth, int srcHeight,
					  boolean handlingTransparency) {

	BufferedImage img = getBufferedImage(image);
	if (img == null) {
	    return true;
	}

	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

        /* The full transform to be applied to the image is the
         * caller's transform concatenated on to the transform
         * from user space to device space. If the caller didn't
         * supply a transform then we just act as if they passed
         * in the identify transform.
         */
        AffineTransform fullTransform = getTransform();
        if (xform == null) {
            xform = new AffineTransform();
        }
        fullTransform.concatenate(xform);

        /* Split the full transform into a pair of
         * transforms. The first transform holds effects
         * that GDI (under Win95) can not perform such
         * as rotation and shearing. The second transform
         * is setup to hold only the scaling effects.
         * These transforms are created such that a point,
         * p, in user space, when transformed by 'fullTransform'
         * lands in the same place as when it is transformed
         * by 'rotTransform' and then 'scaleTransform'.
         *
         * The entire image transformation is not in Java in order
         * to minimize the amount of memory needed in the VM. By
         * dividing the transform in two, we rotate and shear
         * the source image in its own space and only go to
         * the, usually, larger, device space when we ask
         * GDI to perform the final scaling.
         */
        double[] fullMatrix = new double[6];
        fullTransform.getMatrix(fullMatrix);

        /* Calculate the amount of scaling in the x
         * and y directions. This scaling is computed by
         * transforming a unit vector along each axis
         * and computing the resulting magnitude.
         * The computed values 'scaleX' and 'scaleY'
         * represent the amount of scaling GDI will be asked
         * to perform.
         */
        Point2D.Float unitVectorX = new Point2D.Float(1, 0);
        Point2D.Float unitVectorY = new Point2D.Float(0, 1);
        fullTransform.deltaTransform(unitVectorX, unitVectorX);
        fullTransform.deltaTransform(unitVectorY, unitVectorY);

        Point2D.Float origin = new Point2D.Float(0, 0);
        double scaleX = unitVectorX.distance(origin);
        double scaleY = unitVectorY.distance(origin);

        /* We do not need to draw anything if either scaling
         * factor is zero.
         */
        if (scaleX != 0 && scaleY != 0) {

            /* Here's the transformation we will do with Java2D,
            */
            AffineTransform rotTransform = new AffineTransform(
                                        fullMatrix[0] / scaleX,  //m00
                                        fullMatrix[1] / scaleY,  //m10
                                        fullMatrix[2] / scaleX,  //m01
                                        fullMatrix[3] / scaleY,  //m11
                                        fullMatrix[4] / scaleX,  //m02
                                        fullMatrix[5] / scaleY); //m12

            /* The scale transform is not used directly: we instead
             * directly multiply by scaleX and scaleY.
             *
             * Conceptually here is what the scaleTransform is:
             *
             * AffineTransform scaleTransform = new AffineTransform(
             *                      scaleX,                     //m00
             *                      0,                          //m10
             *                      0,                          //m01
             *                      scaleY,                     //m11
             *                      0,                          //m02
             *                      0);                         //m12
             */

            /* Convert the image source's rectangle into the rotated
             * and sheared space. Once there, we calculate a rectangle
             * that encloses the resulting shape. It is this rectangle
             * which defines the size of the BufferedImage we need to
             * create to hold the transformed image.
             */
            Rectangle2D.Float srcRect = new Rectangle2D.Float(srcX, srcY,
                                                              srcWidth,
                                                              srcHeight);

            Shape rotShape = rotTransform.createTransformedShape(srcRect);
            Rectangle2D rotBounds = rotShape.getBounds2D();
	    
	    /* add a fudge factor as some fp precision problems have
             * been observed which caused pixels to be rounded down and
             * out of the image.
             */
	    rotBounds.setRect(rotBounds.getX(), rotBounds.getY(),
                              rotBounds.getWidth()+0.001,
                              rotBounds.getHeight()+0.001);

            int boundsWidth = (int) rotBounds.getWidth();
            int boundsHeight = (int) rotBounds.getHeight();

            if (boundsWidth > 0 && boundsHeight > 0) {

                /* If the image has transparent or semi-transparent
                 * pixels then we'll have the application re-render
                 * the portion of the page covered by the image.
                 * The BufferedImage will be at the image's resolution
                 * to avoid wasting memory. By re-rendering this portion
                 * of a page all compositing is done by Java2D into
                 * the BufferedImage and then that image is copied to
                 * GDI.
 		 * However several special cases can be handled otherwise:
                 * - bitmask transparency with a solid background colour
                 * - images which have transparency color models but no
                 * transparent pixels
                 * - images with bitmask transparency and an IndexColorModel
                 * (the common transparent GIF case) can be handled by
                 * rendering just the opaque pixels.
                 */
                boolean drawOpaque = true;
                if (!handlingTransparency && hasTransparentPixels(img)) {
                    drawOpaque = false;
                    if (isBitmaskTransparency(img)) {
                        if (bgcolor == null) {
                            if (drawBitmaskImage(img, xform, bgcolor,
						 srcX, srcY,
                                                 srcWidth, srcHeight)) {
                                // image drawn, just return.
                                return true;
                            }
                        } else if (bgcolor.getTransparency()
                                   == Transparency.OPAQUE) {
                            drawOpaque = true;
                        }
                    }
		    if (!canDoRedraws()) {
                        drawOpaque = true;
                    }
		} else {
		    // if there's no transparent pixels there's no need
		    // for a background colour. This can avoid edge artifacts
		    // in rotation cases.
		    bgcolor = null;
		}
		// if src region extends beyond the image, the "opaque" path
		// may blit b/g colour (including white) where it shoudn't.
		if ((srcX+srcWidth > img.getWidth(null) ||
		     srcY+srcHeight > img.getHeight(null))
		    && canDoRedraws()) {
		    drawOpaque = false;
		}
                if (drawOpaque == false) {
                    wPrinterJob.saveState(getTransform(), getClip(),
                                           rotBounds, scaleX, scaleY,
                                           srcRect, xform);
		    return true;
                /* The image can be rendered directly by GDI so we
                 * copy it into a BufferedImage (this takes care of
                 * ColorSpace and BufferedImageOp issues) and then
                 * send that to GDI.
                 */
                } else {

                    /* Create a buffered image big enough to hold the portion
                     * of the source image being printed.
                     */
                    BufferedImage deepImage = new BufferedImage(
                                                   (int) rotBounds.getWidth(),
                                                   (int) rotBounds.getHeight(),
                                                 BufferedImage.TYPE_3BYTE_BGR);

                    /* Setup a Graphics2D on to the BufferedImage so that the
                     * source image when copied, lands within the image buffer.
                     */
                    Graphics2D imageGraphics = deepImage.createGraphics();
                    imageGraphics.clipRect(0, 0,
                                           deepImage.getWidth(),
                                           deepImage.getHeight());


                    imageGraphics.translate(-rotBounds.getX(),
                                            -rotBounds.getY());
                    imageGraphics.transform(rotTransform);

                    /* Fill the BufferedImage either with the caller supplied
                     * color, 'bgColor' or, if null, with white.
                     */
                    if (bgcolor == null) {
                        bgcolor = Color.white;
                    }

                    imageGraphics.drawImage(img,
                                            srcX, srcY,
                                            srcX + srcWidth, srcY + srcHeight,
                                            srcX, srcY,
                                            srcX + srcWidth, srcY + srcHeight,
                                            bgcolor, null);

                    /* Because the caller's image has been rotated
                     * and sheared into our BufferedImage and because
                     * we will be handing that BufferedImage directly to
                     * GDI, we need to set an additional clip. This clip
                     * makes sure that only parts of the BufferedImage
                     * that are also part of the caller's image are drawn.
                     */
                    Shape holdClip = getClip();
                    clip(xform.createTransformedShape(srcRect));
		    deviceClip(getClip().getPathIterator(getTransform()));

                    /* Scale the bounding rectangle by the scale transform.
                     * Because the scaling transform has only x and y
                     * scaling components it is equivalent to multiply
                     * the x components of the bounding rectangle by
                     * the x scaling factor and to multiply the y components
                     * by the y scaling factor.
                     */
                    Rectangle2D.Float scaledBounds
                            = new Rectangle2D.Float(
                                    (float) (rotBounds.getX() * scaleX),
                                    (float) (rotBounds.getY() * scaleY),
                                    (float) (rotBounds.getWidth() * scaleX),
                                    (float) (rotBounds.getHeight() * scaleY));

                    /* Pull the raster data from the buffered image
                     * and pass it along to GDI.
                     */
                    ByteComponentRaster tile
                            = (ByteComponentRaster)deepImage.getRaster();

                    wPrinterJob.drawImage3ByteBGR(tile.getDataStorage(),
                                scaledBounds.x, scaledBounds.y,
				(float)Math.rint(scaledBounds.width+0.5),
				(float)Math.rint(scaledBounds.height+0.5),
                                0f, 0f,
				deepImage.getWidth(), deepImage.getHeight());

                    imageGraphics.dispose();
                    setClip(holdClip);
                }
            }
        }

        return true;
    }

    /**
     * Have the printing application redraw everything that falls
     * within the page bounds defined by <code>region</code>.
     */
    public void redrawRegion(Rectangle2D region, double scaleX, double scaleY,
                             Rectangle2D srcRect, AffineTransform xform)
            throws PrinterException {

        WPrinterJob wPrinterJob = (WPrinterJob)getPrinterJob();
        Printable painter = getPrintable();
        PageFormat pageFormat = getPageFormat();
        int pageIndex = getPageIndex();

        /* Create a buffered image big enough to hold the portion
         * of the source image being printed.
         */
        BufferedImage deepImage = new BufferedImage(
                                        (int) region.getWidth(),
                                        (int) region.getHeight(),
                                        BufferedImage.TYPE_3BYTE_BGR);

        /* Get a graphics for the application to render into.
         * We initialize the buffer to white in order to
         * match the paper and then we shift the BufferedImage
         * so that it covers the area on the page where the
         * caller's Image will be drawn.
         */
        Graphics2D g = deepImage.createGraphics();
        ProxyGraphics2D proxy = new ProxyGraphics2D(g, wPrinterJob);
        proxy.setColor(Color.white);
        proxy.fillRect(0, 0, deepImage.getWidth(), deepImage.getHeight());
        proxy.clipRect(0, 0, deepImage.getWidth(), deepImage.getHeight());

        proxy.translate(-region.getX(), -region.getY());

        /* Calculate the resolution of the source image.
         */
        float sourceResX = (float)(wPrinterJob.getXRes() / scaleX);
        float sourceResY = (float)(wPrinterJob.getYRes() / scaleY);

        /* The application expects to see user space at 72 dpi.
         * so change user space from image source resolution to
         *  72 dpi.
         */
        proxy.scale(sourceResX / DEFAULT_USER_RES,
                    sourceResY / DEFAULT_USER_RES);

        proxy.translate(
            -wPrinterJob.getPhysicalPrintableX(pageFormat.getPaper())
               / wPrinterJob.getXRes() * DEFAULT_USER_RES,
            -wPrinterJob.getPhysicalPrintableY(pageFormat.getPaper())
               / wPrinterJob.getYRes() * DEFAULT_USER_RES);
        proxy.transform(new AffineTransform(getPageFormat().getMatrix()));
        proxy.setPaint(Color.black);

        painter.print(proxy, pageFormat, pageIndex);

        g.dispose();

        /* Because the caller's image has been rotated
         * and sheared into our BufferedImage and because
         * we will be handing that BufferedImage directly to
         * GDI, we need to set an additional clip. This clip
         * makes sure that only parts of the BufferedImage
         * that are also part of the caller's image are drawn.
         */
        //Shape holdClip = getClip();
        clip(xform.createTransformedShape(srcRect));
	deviceClip(getClip().getPathIterator(getTransform()));

        /* Scale the bounding rectangle by the scale transform.
         * Because the scaling transform has only x and y
         * scaling components it is equivalent to multiplying
         * the x components of the bounding rectangle by
         * the x scaling factor and to multiplying the y components
         * by the y scaling factor.
         */
        Rectangle2D.Float scaledBounds
                = new Rectangle2D.Float(
                        (float) (region.getX() * scaleX),
                        (float) (region.getY() * scaleY),
                        (float) (region.getWidth() * scaleX),
                        (float) (region.getHeight() * scaleY));

        /* Pull the raster data from the buffered image
         * and pass it along to GDI.
         */
       ByteComponentRaster tile
                = (ByteComponentRaster)deepImage.getRaster();

        wPrinterJob.drawImage3ByteBGR(tile.getDataStorage(),
                    scaledBounds.x, scaledBounds.y,
                    scaledBounds.width,
                    scaledBounds.height,
                    0f, 0f,
                    deepImage.getWidth(), deepImage.getHeight());


        //setClip(holdClip);

    }

    /*
     * Fill the path defined by <code>pathIter</code>
     * with the specified color.
     * The path is provided in device coordinates.
     */
    protected void deviceFill(PathIterator pathIter, Color color) {

	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

	convertToWPath(pathIter);
	wPrinterJob.selectSolidBrush(color);
	wPrinterJob.fillPath();
    }

    /*
     * Set the printer device's clip to be the
     * path defined by <code>pathIter</code>
     * The path is provided in device coordinates.
     */
    protected void deviceClip(PathIterator pathIter) {

	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

	convertToWPath(pathIter);
	wPrinterJob.selectClipPath();
    }

    /**
     * Draw the bounding rectangle using transformed coordinates.
     */
     protected void deviceFrameRect(int x, int y, int width, int height,
     			             Color color) {
		
	AffineTransform deviceTransform = getTransform();
	
	/* check if rotated or sheared */
	int transformType = deviceTransform.getType();
	boolean usePath = ((transformType & 
			   (AffineTransform.TYPE_GENERAL_ROTATION |
			    AffineTransform.TYPE_GENERAL_TRANSFORM)) != 0);
			  
	if (usePath) {
	    draw(new Rectangle2D.Float(x, y, width, height));
	    return;
	}          	
       
	Stroke stroke = getStroke();

       	if (stroke instanceof BasicStroke) {
	    BasicStroke lineStroke = (BasicStroke) stroke;

	    int endCap = lineStroke.getEndCap();
      	    int lineJoin = lineStroke.getLineJoin();
      	    

	    /* check for default style and try to optimize it by 
	     * calling the frameRect native function instead of using paths.
	     */
      	    if ((endCap == BasicStroke.CAP_SQUARE) &&
               	(lineJoin == BasicStroke.JOIN_MITER) &&
               	(lineStroke.getMiterLimit() ==10.0f)) {
		
		float lineWidth = lineStroke.getLineWidth();      	    
		Point2D.Float penSize = new Point2D.Float(lineWidth,
							  lineWidth);

		deviceTransform.deltaTransform(penSize, penSize);
		float deviceLineWidth = Math.min(Math.abs(penSize.x),
						 Math.abs(penSize.y));

		/* transform upper left coordinate */
		Point2D.Float ul_pos = new Point2D.Float(x, y);
		deviceTransform.transform(ul_pos, ul_pos);

		/* transform lower right coordinate */
		Point2D.Float lr_pos = new Point2D.Float(x + width, 
							 y + height);
		deviceTransform.transform(lr_pos, lr_pos);

		float w = (float) (lr_pos.getX() - ul_pos.getX());
		float h = (float)(lr_pos.getY() - ul_pos.getY());
		
		WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

		/* use selectStylePen, if supported */
               	if (wPrinterJob.selectStylePen(endCap, lineJoin,
					   deviceLineWidth, color) == true)  {
		    wPrinterJob.frameRect((float)ul_pos.getX(), 
					  (float)ul_pos.getY(), w, h);
		}
		/* not supported, must be a Win 9x */
               	else {
	
		    double lowerRes = Math.min(wPrinterJob.getXRes(),
					       wPrinterJob.getYRes());

		    if ((deviceLineWidth/lowerRes) < MAX_THINLINE_INCHES) {
			/* use the default pen styles for thin pens. */
            	       	wPrinterJob.selectPen(deviceLineWidth, color);
            	       	wPrinterJob.frameRect((float)ul_pos.getX(),
					      (float)ul_pos.getY(), w, h);
          	    }
          	    else {
			draw(new Rectangle2D.Float(x, y, width, height));
          	    }
        	}
	    }
      	    else {
               	draw(new Rectangle2D.Float(x, y, width, height));
      	    }
	}
     }


     /*    
      * Fill the rectangle with specified color and using Windows'
      * GDI fillRect function.
      * Boundaries are determined by the given coordinates.
      */
    protected void deviceFillRect(int x, int y, int width, int height,
				  Color color) {
	/*
      	 * Transform to device coordinates
   	 */
       	AffineTransform deviceTransform = getTransform();

	/* check if rotated or sheared */
	int transformType = deviceTransform.getType();
	boolean usePath =  ((transformType & 
			       (AffineTransform.TYPE_GENERAL_ROTATION |
				AffineTransform.TYPE_GENERAL_TRANSFORM)) != 0);
	if (usePath) {
	    fill(new Rectangle2D.Float(x, y, width, height));
	    return;
	}

       	Point2D.Float tlc_pos = new Point2D.Float(x, y);
    	deviceTransform.transform(tlc_pos, tlc_pos);

       	Point2D.Float brc_pos = new Point2D.Float(x+width, y+height);
    	deviceTransform.transform(brc_pos, brc_pos);

       	float deviceWidth = (float) (brc_pos.getX() - tlc_pos.getX());
       	float deviceHeight = (float)(brc_pos.getY() - tlc_pos.getY());

       	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();
       	wPrinterJob.fillRect((float)tlc_pos.getX(), (float)tlc_pos.getY(),
			     deviceWidth, deviceHeight, color);
    }


    /**
     * Draw a line using a pen created using the specified color
     * and current stroke properties.
     */
    protected void deviceDrawLine(int xBegin, int yBegin, int xEnd, int yEnd,
  				  Color color) {	    
    	Stroke stroke = getStroke();
       
    	if (stroke instanceof BasicStroke) {
	    BasicStroke lineStroke = (BasicStroke) stroke;
	    
	    if (lineStroke.getDashArray() != null) {
		draw(new Line2D.Float(xBegin, yBegin, xEnd, yEnd));
		return;
	    }

	    float lineWidth = lineStroke.getLineWidth();	    
	    Point2D.Float penSize = new Point2D.Float(lineWidth, lineWidth);

	    AffineTransform deviceTransform = getTransform();
	    deviceTransform.deltaTransform(penSize, penSize);

	    float deviceLineWidth = Math.min(Math.abs(penSize.x),
					     Math.abs(penSize.y));

	    Point2D.Float begin_pos = new Point2D.Float(xBegin, yBegin);
	    deviceTransform.transform(begin_pos, begin_pos);

	    Point2D.Float end_pos = new Point2D.Float(xEnd, yEnd);
	    deviceTransform.transform(end_pos, end_pos);
  
	    int endCap = lineStroke.getEndCap();
	    int lineJoin = lineStroke.getLineJoin();

	    /* check if it's a one-pixel line */
	    if ((end_pos.getX() == begin_pos.getX())
		&& (end_pos.getY() == begin_pos.getY())) {

		/* endCap other than Round will not print!
		 * due to Windows GDI limitation, force it to CAP_ROUND
		 */
		endCap = BasicStroke.CAP_ROUND;
	    }


	    WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

	    /* call native function that creates pen with style */
	    if (wPrinterJob.selectStylePen(endCap, lineJoin, 
					   deviceLineWidth, color)) {
		wPrinterJob.moveTo((float)begin_pos.getX(), 
				   (float)begin_pos.getY());
		wPrinterJob.lineTo((float)end_pos.getX(), 
				   (float)end_pos.getY());
	    }
	    /* selectStylePen is not supported, must be Win 9X */
	    else {

		/* let's see if we can use a a default pen
		 *  if it's round end (Windows' default style)
		 *  or it's vertical/horizontal
		 *  or stroke is too thin.
		 */
		double lowerRes = Math.min(wPrinterJob.getXRes(),
					   wPrinterJob.getYRes());

		if ((endCap == BasicStroke.CAP_ROUND) ||
	         (((xBegin == xEnd) || (yBegin == yEnd)) &&
		 (deviceLineWidth/lowerRes < MAX_THINLINE_INCHES))) {

		    wPrinterJob.selectPen(deviceLineWidth, color);
		    wPrinterJob.moveTo((float)begin_pos.getX(),
				       (float)begin_pos.getY());
		    wPrinterJob.lineTo((float)end_pos.getX(), 
				       (float)end_pos.getY());
        	}
        	else {
		    draw(new Line2D.Float(xBegin, yBegin, xEnd, yEnd));
        	}
	    }
    	}
    }


    /**
     * Given a Java2D <code>PathIterator</code> instance,
     * this method translates that into a Window's path
     * in the printer device context.
     */
    private void convertToWPath(PathIterator pathIter) {

	float[] segment = new float[6];
	int segmentType;

	WPrinterJob wPrinterJob = (WPrinterJob) getPrinterJob();

	/* Map the PathIterator's fill rule into the Window's
	 * polygon fill rule.
	 */
	int polyFillRule;
	if (pathIter.getWindingRule() == PathIterator.WIND_EVEN_ODD) {
	    polyFillRule = WPrinterJob.POLYFILL_ALTERNATE;
	} else {
	    polyFillRule = WPrinterJob.POLYFILL_WINDING;
	}
	wPrinterJob.setPolyFillMode(polyFillRule);

	wPrinterJob.beginPath();

	while (pathIter.isDone() == false) {
	    segmentType = pathIter.currentSegment(segment);

	    switch (segmentType) {
	     case PathIterator.SEG_MOVETO:
		wPrinterJob.moveTo(segment[0], segment[1]);
		break;
	     
	     case PathIterator.SEG_LINETO:
		wPrinterJob.lineTo(segment[0], segment[1]);
		break;

	    /* Convert the quad path to a bezier.
	     */
	     case PathIterator.SEG_QUADTO:
		int lastX = wPrinterJob.getPenX();
		int lastY = wPrinterJob.getPenY();
		float c1x = lastX + (segment[0] - lastX) * 2 / 3;
		float c1y = lastY + (segment[1] - lastY) * 2 / 3;
		float c2x = segment[2] - (segment[2] - segment[0]) * 2/ 3;
		float c2y = segment[3] - (segment[3] - segment[1]) * 2/ 3;
		wPrinterJob.polyBezierTo(c1x, c1y,
				         c2x, c2y,
				         segment[2], segment[3]);
		break;

	     case PathIterator.SEG_CUBICTO:
		wPrinterJob.polyBezierTo(segment[0], segment[1],
				         segment[2], segment[3],
				         segment[4], segment[5]);
		break;

	     case PathIterator.SEG_CLOSE:
		wPrinterJob.closeFigure();
		break;
	    }


	    pathIter.next();
	}

	wPrinterJob.endPath();

    }

}
