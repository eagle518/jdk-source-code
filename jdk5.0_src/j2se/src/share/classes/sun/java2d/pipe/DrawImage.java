/*
 * @(#)DrawImage.java	1.30 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DirectColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import sun.awt.SunHints;
import sun.awt.image.ImageRepresentation;
import sun.awt.image.RemoteOffScreenImage;
import sun.awt.image.ToolkitImage;
import sun.java2d.InvalidPipeException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.Blit;
import sun.java2d.loops.BlitBg;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.ScaledBlit;
import sun.java2d.loops.SurfaceType;

public class DrawImage implements DrawImagePipe
{
    public boolean copyImage(SunGraphics2D sg, Image img,
			     int x, int y,
			     Color bgColor)
    {
	int imgw = img.getWidth(null);
	int imgh = img.getHeight(null);
	if (isSimpleTranslate(sg)) {
	    return renderImageCopy(sg, img, bgColor,
				   x + sg.transX, y + sg.transY,
				   0, 0, imgw, imgh);
	}
	AffineTransform atfm = sg.transform;
	if ((x | y) != 0) {
	    atfm = new AffineTransform(atfm);
	    atfm.translate(x, y);
	}
	transformImage(sg, img, atfm, sg.interpolationType,
		       0, 0, imgw, imgh, bgColor);
	return true;
    }

    public boolean copyImage(SunGraphics2D sg, Image img,
			     int dx, int dy, int sx, int sy, int w, int h,
			     Color bgColor)
    {
	if (isSimpleTranslate(sg)) {
	    return renderImageCopy(sg, img, bgColor,
				   dx + sg.transX, dy + sg.transY,
				   sx, sy, w, h);
	}
	scaleImage(sg, img, dx, dy, (dx + w), (dy + h),
		   sx, sy, (sx + w), (sy + h), bgColor);
	return true;
    }

    public boolean scaleImage(SunGraphics2D sg, Image img, int x, int y,
			      int width, int height,
 			      Color bgColor)
    {
	int imgw = img.getWidth(null);
	int imgh = img.getHeight(null);
	// Only accelerate scale if:
	//	    - w/h positive values
	//	    - sg transform integer translate/identity only
	//	    - no bgColor in operation
	if ((width > 0) && (height > 0) && isSimpleTranslate(sg)) {
	    if (renderImageScale(sg, img, bgColor,
				 x + sg.transX, y + sg.transY,
				 0, 0,
				 width, height, imgw, imgh))
	    {
		return true;
	    }
	}

	AffineTransform atfm = sg.transform;
	if ((x | y) != 0 || width != imgw || height != imgh) {
	    atfm = new AffineTransform(atfm);
	    atfm.translate(x, y);
	    atfm.scale(((double)width)/imgw, ((double)height)/imgh);
	}
	transformImage(sg, img, atfm, sg.interpolationType,
		       0, 0, imgw, imgh, bgColor);
	return true;
    }

    /*
     * This method is only called in those circumstances where the
     * operation has a non-null secondary transform specfied.  Its
     * role is to check for various optimizations based on the types
     * of both the secondary and SG2D transforms and to do some
     * quick calculations to avoid having to combine the transforms
     * and/or to call a more generalized method.
     */
    protected void transformImage(SunGraphics2D sg, Image img, int x, int y,
				  AffineTransform extraAT, int interpType)
    {
	int txtype = extraAT.getType();
	int imgw = img.getWidth(null);
	int imgh = img.getHeight(null);
	boolean checkfinalxform;

	if (sg.transformState <= sg.TRANSFORM_ANY_TRANSLATE &&
	    (txtype == AffineTransform.TYPE_IDENTITY ||
	     txtype == AffineTransform.TYPE_TRANSLATION))
	{
	    // First optimization - both are some kind of translate

	    // Combine the translations and check if interpolation is necessary.
	    double tx = extraAT.getTranslateX();
	    double ty = extraAT.getTranslateY();
	    tx += sg.transform.getTranslateX();
	    ty += sg.transform.getTranslateY();
	    int itx = (int) Math.floor(tx + 0.5);
	    int ity = (int) Math.floor(ty + 0.5);
	    if (interpType == AffineTransformOp.TYPE_NEAREST_NEIGHBOR ||
		(closeToInteger(itx, tx) && closeToInteger(ity, ty)))
	    {
		renderImageCopy(sg, img, null, x+itx, y+ity, 0, 0, imgw, imgh);
		return;
	    }
	    checkfinalxform = false;
	} else if (sg.transformState <= sg.TRANSFORM_TRANSLATESCALE &&
		   ((txtype & (AffineTransform.TYPE_FLIP |
			       AffineTransform.TYPE_MASK_ROTATION |
			       AffineTransform.TYPE_GENERAL_TRANSFORM)) == 0))
	{
	    // Second optimization - both are some kind of translate or scale

	    // Combine the scales and check if interpolation is necessary.

	    // Transform source bounds by extraAT,
	    // then translate the bounds again by x, y
	    // then transform the bounds again by sg.transform
	    double coords[] = new double[] {
		0, 0, imgw, imgh,
	    };
	    extraAT.transform(coords, 0, coords, 0, 2);
	    coords[0] += x;
	    coords[1] += y;
	    coords[2] += x;
	    coords[3] += y;
	    sg.transform.transform(coords, 0, coords, 0, 2);

	    if (tryCopyOrScale(sg, img, 0, 0, imgw, imgh,
			       null, interpType, coords))
	    {
		return;
	    }
	    checkfinalxform = false;
	} else {
	    checkfinalxform = true;
	}

        // Begin Transform
        AffineTransform tx = new AffineTransform(sg.transform);
	tx.translate(x, y);
	tx.concatenate(extraAT);

	// Do not try any more optimizations if either of the cases
	// above was tried as we have already verified that the
	// resulting transform will not simplify.
	if (checkfinalxform) {
	    // In this case neither of the above simple transform
	    // pairs was found so we will do some final tests on
	    // the final rendering transform which may be the
	    // simple product of two complex transforms.
	    transformImage(sg, img, tx, interpType, 0, 0, imgw, imgh, null);
	} else {
	    renderImageXform(sg, img, tx, interpType, 0, 0, imgw, imgh, null);
	}
    }

    /*
     * This method is called with a final rendering transform that
     * has combined all of the information about the Graphics2D
     * transform attribute with the transformations specified by
     * the arguments to the drawImage call.
     * Its role is to see if the combined transform ends up being
     * acceleratable by either a renderImageCopy or renderImageScale
     * once all of the math is done.
     */
    protected void transformImage(SunGraphics2D sg, Image img,
				  AffineTransform tx, int interpType,
				  int sx1, int sy1, int sx2, int sy2,
				  Color bgColor)
    {
	// Transform 3 source corners by tx and analyze them
	// for simplified operations (Copy or Scale).  Using
	// 3 points lets us analyze any kind of transform,
	// even transforms that involve very tiny amounts of
	// rotation or skew to see if they degenerate to a
	// simple scale or copy operation within the allowable
	// error bounds.
	double coords[] = new double[] {
	    sx1, sy1,	// upper left
	    sx2, sy2,	// lower right
	    sx1, sy2,	// lower left
	};
	tx.transform(coords, 0, coords, 0, 3);
	// First test if the X coords of the transformed UL
	// and LL points match and that the Y coords of the
	// transformed LR and LL points also match.
	// If they do then it is a "rectilinear" transform and
	// tryCopyOrScale will make sure it is upright and
	// integer-based.
	if (Math.abs(coords[0] - coords[4]) < MAX_TX_ERROR &&
	    Math.abs(coords[3] - coords[5]) < MAX_TX_ERROR &&
	    tryCopyOrScale(sg, img, sx1, sy1, sx2-sx1, sy2-sy1,
			   bgColor, interpType, coords))
	{
	    return;
	}

	renderImageXform(sg, img, tx, interpType, sx1, sy1, sx2, sy2, bgColor);
    }

    /*
     * Check the bounding coordinates of the transformed source
     * image to see if they fall on integer coordinates such
     * that they will cause no interpolation anomalies if we
     * use our simplified Blit or ScaledBlit operations instead
     * of a full transform operation.
     */
    protected boolean tryCopyOrScale(SunGraphics2D sg,
				     Image img,
				     int imgx, int imgy,
				     int imgw, int imgh,
				     Color bgColor, int interpType,
				     double coords[])
    {
	// First check if width and height are close to integers.
	// We can only optimize to renderCopy or renderScale if
	// the width and height work out very close to integers,
	// otherwise the non-integer scale might cause slightly
	// different pixel choices even with NEAREST_NEIGHBOR.
	double dx = coords[0];
	double dy = coords[1];
	double dw = coords[2] - dx;
	double dh = coords[3] - dy;
	int idw = (int) Math.floor(dw + 0.5);
	int idh = (int) Math.floor(dh + 0.5);
	if (closeToInteger(idw, dw) && closeToInteger(idh, dh)) {
	    if (idw <= 0 || idh <= 0) {
		return (idw == 0 || idh == 0);
	    }
	    // Round location to nearest pixel and then test
	    // if it will cause interpolation anomalies.
	    int idx = (int) Math.floor(dx + 0.5);
	    int idy = (int) Math.floor(dy + 0.5);
	    if (interpType == AffineTransformOp.TYPE_NEAREST_NEIGHBOR ||
		(closeToInteger(idx, dx) && closeToInteger(idy, dy)))
	    {
		if (idw == imgw && idh == imgh) {
		    renderImageCopy(sg, img, bgColor,
				    idx, idy, imgx, imgy,
				    imgw, imgh);
		    return true;
		}
		if (renderImageScale(sg, img, bgColor,
				     idx, idy, imgx, imgy,
				     idw, idh, imgw, imgh))
		{
		    return true;
		}
	    }
	}
	return false;
    }

    protected void renderImageXform(SunGraphics2D sg, Image img,
				    AffineTransform tx, int interpType,
				    int sx1, int sy1, int sx2, int sy2,
				    Color bgColor)
    {
	// Must not have been acceleratable as a simple scale - do this
	// as a general transform
	BufferedImage bImg = getBufferedImage(img);
        int bImgWidth = bImg.getWidth();
        int bImgHeight = bImg.getHeight();

	if ((sx1 | sy1) != 0 || sx2 < bImgWidth || sy2 < bImgHeight) {
            // The actual semantics of this method is to draw the image
            // up to, but not including the width and height.  We need
            // to include the width and height in the source image so
            // that the interpolation will have enough data to fill in
            // the last pixel(s).  We subtract the last point in the
            // bounding box rectangle in renderingPipeImage.
	    sx2 -= sx1;
	    sy2 -= sy1;
	    sx1 += bImg.getMinX();
	    sy1 += bImg.getMinY();
            bImg = bImg.getSubimage(sx1, sy1, sx2, sy2);
	    bImgWidth = sx2;
	    bImgHeight = sy2;
	}

	Region clip = sg.getCompClip();

        {
            // MediaLib/AffineTransformOp handle indexed images poorly in
            // the bilinear case, so we will convert the source image into
            // an INT_ARGB_PRE image (MediaLib produces better results with
            // premultiplied alpha)
            // REMIND: this and other similar special case hacks should go
            // away in the future...
            if ((interpType != AffineTransformOp.TYPE_NEAREST_NEIGHBOR) &&
                (bImg.getColorModel() instanceof IndexColorModel))
            {
                BufferedImage tmpBI =
                    new BufferedImage(bImgWidth, bImgHeight,
                                      BufferedImage.TYPE_INT_ARGB_PRE);
                Graphics2D g2d = tmpBI.createGraphics();

                try {
                    g2d.setComposite(AlphaComposite.Src);
                    g2d.drawImage(bImg, 0, 0, null);
                } finally {
                    g2d.dispose();
                }

                bImg = tmpBI;
            }

            ColorModel dstCM = getTransformColorModel(sg, bImg, tx);
            AffineTransformOp affOp = new AffineTransformOp(tx, interpType);
            // Transform the bounding box of the image in order to
            // extract the translation and to figure out the destination
            // width and height
            Rectangle bounds = affOp.getBounds2D(bImg).getBounds();
            clip = clip.getBoundsIntersection(bounds);
            if (clip.isEmpty()) {
		return;
            }

            int x = clip.getLoX();
            int y = clip.getLoY();
            bImgWidth  = clip.getWidth();
            bImgHeight = clip.getHeight();

            if ((x | y) != 0) {
		AffineTransform tx2;
		tx2 = AffineTransform.getTranslateInstance(-x, -y);
		tx2.concatenate(tx);
                affOp = new AffineTransformOp(tx2, interpType);
            }

            // Create a new dst image.
            BufferedImage dst;
            if (dstCM == bImg.getColorModel()) {
                // It is better to use Raster.createCompatibleWritableRaster
                // because the equivalent call in ColorModel cannot
                // figure out if the image is, for example, RGB vs BGR.
                // This is the fix for bug 4245031.
                dst = new BufferedImage(dstCM,
                  bImg.getRaster().createCompatibleWritableRaster(bImgWidth,
                                                                  bImgHeight),
                                        dstCM.isAlphaPremultiplied(), null);
            }
            else {
                dst = new BufferedImage(dstCM,
                             dstCM.createCompatibleWritableRaster(bImgWidth,
                                                                  bImgHeight),
                             dstCM.isAlphaPremultiplied(), null);
            }

            // Now transform the image by the new transform.
            bImg = affOp.filter(bImg, dst);
            if (bImg == null) {
                return;
            }
        }

	/* REMIND: Should avoid a getSubImage above by preserving srcx, srcy */
	/* REMIND: bgColor needs to be handled before transform! */
	renderImageCopy(sg, bImg, bgColor,
			clip.getLoX(), clip.getLoY(),
			0, 0,
			clip.getWidth(), clip.getHeight());
    }

    // Render an image using only integer translation
    // (no scale or transform or sub-pixel interpolated translations).
    protected boolean renderImageCopy(SunGraphics2D sg, Image img,
				      Color bgColor,
				      int dx, int dy,
				      int sx, int sy,
				      int w, int h)
    {
	Region clip = sg.getCompClip();
	SurfaceData sData =
	    SurfaceData.getSourceSurfaceData(img, sg.surfaceData,
					     sg.imageComp,
					     bgColor, false);
	if (sData == null) {
	    return false;
	}

	int attempts = 0;
	// Loop up to twice through; this gives us a chance to
	// revalidate the surfaceData objects in case of an exception
	// and try it once more
	while (true) {
	    try {
		SurfaceType src = sData.getSurfaceType();
		SurfaceType dst = sg.surfaceData.getSurfaceType();
		blitSurfaceData(sg, clip,
				sData, sg.surfaceData, src, dst,
				sx, sy, dx, dy, w, h, bgColor);
		return true;
	    } catch (NullPointerException e) {
		if (!(SurfaceData.isNull(sg.surfaceData) || 
		      SurfaceData.isNull(sData)))
		{
		    // Something else caused the exception, throw it...
		    throw e;
		}
		return false;
		// NOP if we have been disposed
	    } catch (InvalidPipeException e) {
		// Always catch the exception; try this a couple of times
		// and fail silently if the system is not yet ready to
		// revalidate the source or dest surfaceData objects.
		++attempts;
		sData = sData.getReplacement();
		clip = sg.getCompClip();   // ensures sg.surfaceData is valid
		if (SurfaceData.isNull(sg.surfaceData) ||
		    SurfaceData.isNull(sData) || (attempts > 1))
		{
		    return false;
		}
	    }
	}
    }

    // Render an image using only integer scaling (no transform).
    protected boolean renderImageScale(SunGraphics2D sg, Image img,
				       Color bgColor,
				       int dx, int dy, int sx, int sy,
				       int dw, int dh, int sw, int sh)
    {
	// Currently only NEAREST_NEIGHBOR interpolation is implemented
	// for ScaledBlit operations.
	if (sg.interpolationType != AffineTransformOp.TYPE_NEAREST_NEIGHBOR) {
	    return false;
	}

	Region clip = sg.getCompClip();
	SurfaceData sData =
	    SurfaceData.getSourceSurfaceData(img, sg.surfaceData,
					     sg.imageComp,
					     bgColor, true);

	if (sData == null || isBgOperation(sData, bgColor)) {
	    return false;
	}

	int attempts = 0;
	// Loop up to twice through; this gives us a chance to
	// revalidate the surfaceData objects in case of an exception
	// and try it once more
	while (true) {
	    try {
		SurfaceType src = sData.getSurfaceType();
		SurfaceType dst = sg.surfaceData.getSurfaceType();
		return scaleSurfaceData(sg, clip,
					sData, sg.surfaceData, src, dst,
					dx, dy, sx, sy,
					dw, dh, sw, sh);
	    } catch (NullPointerException e) {
		if (!SurfaceData.isNull(sg.surfaceData)) {
		    // Something else caused the exception, throw it...
		    throw e;
		}
		return false;
		// NOP if we have been disposed
	    } catch (InvalidPipeException e) {
		// Always catch the exception; try this a couple of times
		// and fail silently if the system is not yet ready to
		// revalidate the source or dest surfaceData objects.
		++attempts;
		sData = sData.getReplacement();
		clip = sg.getCompClip();  // ensures sg.surfaceData is valid
		if (SurfaceData.isNull(sg.surfaceData) ||
		    SurfaceData.isNull(sData) || (attempts > 1))
		{
		    return false;
		}
	    }
	}
    }

    public boolean scaleImage(SunGraphics2D sg, Image img,
			      int dx1, int dy1, int dx2, int dy2,
			      int sx1, int sy1, int sx2, int sy2,
			      Color bgColor)
    {
	int srcW, srcH, dstW, dstH;
	int srcX, srcY, dstX, dstY;
	boolean srcWidthFlip = false;
	boolean srcHeightFlip = false;
	boolean dstWidthFlip = false;
	boolean dstHeightFlip = false;

	if (sx2 > sx1) {
	    srcW = sx2 - sx1;
	    srcX = sx1;
	} else {
	    srcWidthFlip = true;
	    srcW = sx1 - sx2;
	    srcX = sx2;
	}
	if (sy2 > sy1) {
	    srcH = sy2-sy1;
	    srcY = sy1;
	} else {
	    srcHeightFlip = true;
	    srcH = sy1-sy2;
	    srcY = sy2;
	}
	if (dx2 > dx1) {
	    dstW = dx2 - dx1;
	    dstX = dx1;
	} else {
	    dstW = dx1 - dx2;
	    dstWidthFlip = true;
	    dstX = dx2;
	}
	if (dy2 > dy1) {
	    dstH = dy2 - dy1;
	    dstY = dy1;
	} else {
	    dstH = dy1 - dy2;
	    dstHeightFlip = true;
	    dstY = dy2;
	}
	if (srcW <= 0 || srcH <= 0) {
	    return true;
	}
	// Only accelerate scale if it does not involve a flip or transform
	if ((srcWidthFlip == dstWidthFlip) &&
	    (srcHeightFlip == dstHeightFlip) &&
	    isSimpleTranslate(sg))
	{
	    if (renderImageScale(sg, img, bgColor,
				 dstX + sg.transX, dstY + sg.transY,
				 srcX, srcY,
				 dstW, dstH,
				 srcW, srcH))
	    {
		return true;
	    }
	}

	AffineTransform atfm = new AffineTransform(sg.transform);
	atfm.translate(dx1, dy1);
	double m00 = (double)(dx2-dx1)/(sx2-sx1);
	double m11 = (double)(dy2-dy1)/(sy2-sy1);
	atfm.scale(m00, m11);
	atfm.translate(srcX-sx1, srcY-sy1);

	int imgW = img.getWidth(null);
	int imgH = img.getHeight(null);
	srcW += srcX;
	srcH += srcY;
	// Make sure we are not out of bounds
	if (srcW > imgW) {
	    srcW = imgW;
	}
	if (srcH > imgH) {
	    srcH = imgH;
	}
	if (srcX < 0) {
	    atfm.translate(-srcX, 0);
	    srcX = 0;
	}
	if (srcY < 0) {
	    atfm.translate(0, -srcY);
	    srcY = 0;
	}
	if (srcX >= srcW || srcY >= srcH) {
	    return true;
	}

        transformImage(sg, img, atfm, sg.interpolationType,
		       srcX, srcY, srcW, srcH, bgColor);
	return true;
    }

    /**
     ** Utilities
     ** The following methods are used by the public methods above
     ** for performing various operations
     **/

    /*
     * This constant represents a tradeoff between the
     * need to make sure that image transformations are
     * "very close" to integer device coordinates before
     * we decide to use an integer scale or copy operation
     * as a substitute and the fact that roundoff errors
     * in AffineTransforms are frequently introduced by
     * performing multiple sequential operations on them.
     *
     * The evaluation of bug 4990624 details the potential
     * for this error cutoff to result in display anomalies
     * in different types of image operations and how this
     * value represents a good compromise here.
     */
    private static final double MAX_TX_ERROR = .0001;

    public static boolean closeToInteger(int i, double d) {
	return (Math.abs(d-i) < MAX_TX_ERROR);
    }

    public static boolean isSimpleTranslate(SunGraphics2D sg) {
	int ts = sg.transformState;
	if (ts <= sg.TRANSFORM_INT_TRANSLATE) {
	    // Integer translates are always "simple"
	    return true;
	}
	if (ts >= sg.TRANSFORM_TRANSLATESCALE) {
	    // Scales and beyond are always "not simple"
	    return false;
	}
	// non-integer translates are only simple when not interpolating
	if (sg.interpolationType == AffineTransformOp.TYPE_NEAREST_NEIGHBOR) {
	    return true;
	}
	return false;
    }

    protected static boolean isBgOperation(SurfaceData srcData, Color bgColor) {
	// If we cannot get the sData, then cannot assume anything about
	// the image
	return ((srcData == null) ||
		((bgColor != null) &&
		 (srcData.getTransparency() != Transparency.OPAQUE)));
    }

    protected BufferedImage getBufferedImage(Image img) {
        if (img instanceof RemoteOffScreenImage) {
            return ((RemoteOffScreenImage)img).getSnapshot();
        } else if (img instanceof BufferedImage) {
	    return (BufferedImage)img;
	}
	// Must be VolatileImage; get BufferedImage representation
	return ((VolatileImage)img).getSnapshot();
    }

    /*
     * Return the color model to be used with this BufferedImage and
     * transform.
     */
    private ColorModel getTransformColorModel(SunGraphics2D sg,
					      BufferedImage bImg,
                                              AffineTransform tx) {
        ColorModel cm = bImg.getColorModel();
        ColorModel dstCM = cm;

        if (tx.isIdentity()) {
            return dstCM;
        }
        int type = tx.getType();
        boolean needTrans =
            ((type&(tx.TYPE_MASK_ROTATION|tx.TYPE_GENERAL_TRANSFORM)) != 0);
        if (! needTrans && type != tx.TYPE_TRANSLATION && type != tx.TYPE_IDENTITY)
        {
            double[] mtx = new double[4];
            tx.getMatrix(mtx);
            // Check out the matrix.  A non-integral scale will force ARGB
            // since the edge conditions cannot be guaranteed.
            needTrans = (mtx[0] != (int)mtx[0] || mtx[3] != (int)mtx[3]);
        }

        if (sg.renderHint != SunHints.INTVAL_RENDER_QUALITY) {
            if (cm instanceof IndexColorModel) {
                Raster raster = bImg.getRaster();
                IndexColorModel icm = (IndexColorModel) cm;
                // Just need to make sure that we have a transparent pixel
                if (needTrans && cm.getTransparency() == cm.OPAQUE) {
                    // Fix 4221407
                    if (raster instanceof sun.awt.image.BytePackedRaster) {
                        dstCM = ColorModel.getRGBdefault();
                    }
                    else {
                        double[] matrix = new double[6];
                        tx.getMatrix(matrix);
                        if (matrix[1] == 0. && matrix[2] ==0.
                            && matrix[4] == 0. && matrix[5] == 0.) {
                            // Only scaling so do not need to create
                        }
                        else {
                            int mapSize = icm.getMapSize();
                            if (mapSize < 256) {
                                int[] cmap = new int[mapSize+1];
                                icm.getRGBs(cmap);
                                cmap[mapSize] = 0x0000;
                                dstCM = new
                                    IndexColorModel(icm.getPixelSize(),
                                                    mapSize+1,
                                                    cmap, 0, true, mapSize,
                                                    DataBuffer.TYPE_BYTE);
                            }
                            else {
                                dstCM = ColorModel.getRGBdefault();
                            }
                        }  /* if (matrix[0] < 1.f ...) */
                    }   /* raster instanceof sun.awt.image.BytePackedRaster */
                } /* if (cm.getTransparency() == cm.OPAQUE) */
            } /* if (cm instanceof IndexColorModel) */
            else if (needTrans && cm.getTransparency() == cm.OPAQUE) {
                // Need a bitmask transparency
                // REMIND: for now, use full transparency since no loops
                // for bitmask
                dstCM = ColorModel.getRGBdefault();
            }
        } /* if (sg.renderHint == RENDER_QUALITY) */
        else {

            if (cm instanceof IndexColorModel ||
                (needTrans && cm.getTransparency() == cm.OPAQUE))
            {
                // Need a bitmask transparency
                // REMIND: for now, use full transparency since no loops
                // for bitmask
                dstCM = ColorModel.getRGBdefault();
            }
        }

        return dstCM;
    }

    protected void blitSurfaceData(SunGraphics2D sg,
				   Region clipRegion,
				   SurfaceData srcData,
				   SurfaceData dstData,
				   SurfaceType srcType,
				   SurfaceType dstType,
				   int sx, int sy, int dx, int dy,
				   int w, int h,
				   Color bgColor)
    {
	if (w <= 0 || h <= 0) {
	    /*
	     * Fix for bugid 4783274 - BlitBg throws an exception for
	     * a particular set of anomalous parameters.
	     * REMIND: The native loops do proper clipping and would
	     * detect this situation themselves, but the Java loops
	     * all seem to trust their parameters a little too well
	     * to the point where they will try to process a negative
	     * area of pixels and throw exceptions.  The real fix is
	     * to modify the Java loops to do proper clipping so that
	     * they can deal with negative dimensions as well as
	     * improperly large dimensions, but that fix is too risky
	     * to integrate for Mantis at this point.  In the meantime
	     * eliminating the negative or zero dimensions here is
	     * "correct" and saves them from some nasty exceptional
	     * conditions, one of which is the test case of 4783274.
	     */
	    return;
	}
	CompositeType comp = sg.imageComp;
	if (CompositeType.SrcOverNoEa.equals(comp) &&
	    (srcData.getTransparency() == Transparency.OPAQUE ||
	     (bgColor != null &&
	      bgColor.getTransparency() == Transparency.OPAQUE)))
	{
	    comp = CompositeType.SrcNoEa;
	}
	if (!isBgOperation(srcData, bgColor)) {
	    Blit blit = Blit.getFromCache(srcType, comp, dstType);
            blit.Blit(srcData, dstData, sg.composite, clipRegion,
		      sx, sy, dx, dy, w, h);
	} else {
	    BlitBg blit = BlitBg.getFromCache(srcType, comp, dstType);
            blit.BlitBg(srcData, dstData, sg.composite, clipRegion,
			bgColor, sx, sy, dx, dy, w, h);
	}
    }

    protected boolean scaleSurfaceData(SunGraphics2D sg,
				       Region clipRegion,
				       SurfaceData srcData,
				       SurfaceData dstData,
				       SurfaceType srcType,
				       SurfaceType dstType,
				       int dx, int dy, int sx, int sy,
				       int dw, int dh, int sw, int sh)
    {
	CompositeType comp = sg.imageComp;
	if (CompositeType.SrcOverNoEa.equals(comp) &&
	    (srcData.getTransparency() == Transparency.OPAQUE))
	{
	    comp = CompositeType.SrcNoEa;
	}

	ScaledBlit blit = ScaledBlit.getFromCache(srcType, comp, dstType);
	if (blit != null) {
	    blit.Scale(srcData, dstData, sg.composite, clipRegion,
		       sx, sy, dx, dy, sw, sh, dw, dh);
	    return true;
	}
	return false;
    }

    protected static boolean imageReady(ToolkitImage sunimg,
					ImageObserver observer)
    {
	if (sunimg.hasError()) {
	    if (observer != null) {
		observer.imageUpdate(sunimg,
				     ImageObserver.ERROR|ImageObserver.ABORT,
				     -1, -1, -1, -1);
	    }
	    return false;
	}
	return true;
    }

    public boolean copyImage(SunGraphics2D sg, Image img,
			     int x, int y,
			     Color bgColor,
			     ImageObserver observer) {
	if (!(img instanceof ToolkitImage)) {
	    return copyImage(sg, img, x, y, bgColor);
	} else {
	    ToolkitImage sunimg = (ToolkitImage)img;
	    if (!imageReady(sunimg, observer)) {
		return false;
	    }
	    ImageRepresentation ir = sunimg.getImageRep();
	    return ir.drawToBufImage(sg, sunimg, x, y, bgColor, observer);
	}
    }

    public boolean copyImage(SunGraphics2D sg, Image img,
			     int dx, int dy, int sx, int sy, int w, int h,
			     Color bgColor,
			     ImageObserver observer) {
	if (!(img instanceof ToolkitImage)) {
	    return copyImage(sg, img, dx, dy, sx, sy, w, h, bgColor);
	} else {
	    ToolkitImage sunimg = (ToolkitImage)img;
	    if (!imageReady(sunimg, observer)) {
		return false;
	    }
	    ImageRepresentation ir = sunimg.getImageRep();
	    return ir.drawToBufImage(sg, sunimg,
				     dx, dy, (dx + w), (dy + h),
				     sx, sy, (sx + w), (sy + h),
				     bgColor, observer);
	}
    }

    public boolean scaleImage(SunGraphics2D sg, Image img,
				int x, int y,
				int width, int height,
				Color bgColor,
 				ImageObserver observer) {
	if (!(img instanceof ToolkitImage)) {
	    return scaleImage(sg, img, x, y, width, height, bgColor);
	} else {
	    ToolkitImage sunimg = (ToolkitImage)img;
	    if (!imageReady(sunimg, observer)) {
		return false;
	    }
	    ImageRepresentation ir = sunimg.getImageRep();
	    return ir.drawToBufImage(sg, sunimg, x, y, width, height, bgColor,
				     observer);
	}
    }

    public boolean scaleImage(SunGraphics2D sg, Image img,
			      int dx1, int dy1, int dx2, int dy2,
			      int sx1, int sy1, int sx2, int sy2,
			      Color bgColor,
			      ImageObserver observer) {
	if (!(img instanceof ToolkitImage)) {
	    return scaleImage(sg, img, dx1, dy1, dx2, dy2,
			      sx1, sy1, sx2, sy2, bgColor);
	} else {
	    ToolkitImage sunimg = (ToolkitImage)img;
	    if (!imageReady(sunimg, observer)) {
		return false;
	    }
	    ImageRepresentation ir = sunimg.getImageRep();
	    return ir.drawToBufImage(sg, sunimg, dx1, dy1, dx2, dy2,
				     sx1, sy1, sx2, sy2, bgColor, observer);
	}
    }

    public boolean transformImage(SunGraphics2D sg, Image img,
				  AffineTransform atfm,
				  ImageObserver observer) {
	if (!(img instanceof ToolkitImage)) {
	    transformImage(sg, img, 0, 0, atfm, sg.interpolationType);
	    return true;
	} else {
	    ToolkitImage sunimg = (ToolkitImage)img;
	    if (!imageReady(sunimg, observer)) {
		return false;
	    }
	    ImageRepresentation ir = sunimg.getImageRep();
	    return ir.drawToBufImage(sg, sunimg, atfm, observer);
	}
    }

    public void transformImage(SunGraphics2D sg, BufferedImage img,
			       BufferedImageOp op, int x, int y)
    {
        if (op != null) {
            if (op instanceof AffineTransformOp) {
                AffineTransformOp atop = (AffineTransformOp) op;
		transformImage(sg, img, x, y,
			       atop.getTransform(),
			       atop.getInterpolationType());
		return;
            } else {
                img = op.filter(img, null);
            }
        }
	copyImage(sg, img, x, y, null);
    }
}
