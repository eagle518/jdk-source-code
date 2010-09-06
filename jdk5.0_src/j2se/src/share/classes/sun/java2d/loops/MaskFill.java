/*
 * @(#)MaskFill.java	1.16 04/03/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.loops;

import java.awt.Paint;
import java.awt.PaintContext;
import java.awt.Composite;
import java.awt.Rectangle;
import java.awt.image.ColorModel;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import sun.awt.image.BufImgSurfaceData;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;

/**
 * MaskFill
 * 1) fills rectangles of pixels on a surface
 * 2) performs compositing of colors based upon a Composite
 *    parameter
 * 3) blends result of composite with destination using an
 *    alpha coverage mask
 * 4) the mask may be null in which case it should be treated
 *    as if it were an array of all opaque values (0xff)
 */
public class MaskFill extends GraphicsPrimitive
{
    public static final String methodSignature = "MaskFill(...)".toString();

    public static final int primTypeID = makePrimTypeID(); 

    private static RenderCache fillcache = new RenderCache(10);

    public static MaskFill locate(SurfaceType srctype,
				  CompositeType comptype,
				  SurfaceType dsttype)
    {
	return (MaskFill)
	    GraphicsPrimitiveMgr.locate(primTypeID,
					srctype, comptype, dsttype);
    }

    public static MaskFill locatePrim(SurfaceType srctype,
				      CompositeType comptype,
				      SurfaceType dsttype)
    {
	return (MaskFill)
	    GraphicsPrimitiveMgr.locatePrim(primTypeID,
					    srctype, comptype, dsttype);
    }

    /*
     * Note that this uses locatePrim, not locate, so it can return
     * null if there is no specific loop to handle this op...
     */
    public static MaskFill getFromCache(SurfaceType src,
					CompositeType comp,
					SurfaceType dst)
    {
	Object o = fillcache.get(src, comp, dst);
	if (o != null) {
	    return (MaskFill) o;
	}
	MaskFill fill = locatePrim(src, comp, dst);
	if (fill != null) {
	    fillcache.put(src, comp, dst, fill);
	}
	return fill;
    }

    protected MaskFill(SurfaceType srctype,
		       CompositeType comptype,
		       SurfaceType dsttype)
    {
	super(methodSignature, primTypeID, srctype, comptype, dsttype);
    }

    public MaskFill(long pNativePrim,
		    SurfaceType srctype,
		    CompositeType comptype,
		    SurfaceType dsttype)
    {
	super(pNativePrim, methodSignature, primTypeID, srctype, comptype, dsttype);
    }

    /**
     * All MaskFill implementors must have this invoker method
     */
    public native void MaskFill(SunGraphics2D sg2d, SurfaceData sData,
				Composite comp,
				int x, int y, int w, int h,
				byte[] mask, int maskoff, int maskscan);

    static {
	GraphicsPrimitiveMgr.registerGeneral(new MaskFill(null, null, null));
    }

    public GraphicsPrimitive makePrimitive(SurfaceType srctype,
					   CompositeType comptype,
					   SurfaceType dsttype)
    {
	if (SurfaceType.OpaqueColor.equals(srctype) ||
	    SurfaceType.AnyColor.equals(srctype))
	{
            if (CompositeType.Xor.equals(comptype)) {
                throw new InternalError("Cannot construct MaskFill for " +
                                        "XOR mode");
            } else {
                return new General(srctype, comptype, dsttype);
            }
	} else {
	    throw new InternalError("MaskFill can only fill with colors");
	}
    }

    private static class General extends MaskFill {
	FillRect fillop;
	MaskBlit maskop;

	public General(SurfaceType srctype,
		       CompositeType comptype,
		       SurfaceType dsttype)
	{
	    super(srctype, comptype, dsttype);
	    fillop = FillRect.locate(srctype,
				     CompositeType.SrcNoEa,
				     SurfaceType.IntArgb);
	    maskop = MaskBlit.locate(SurfaceType.IntArgb, comptype, dsttype);
	}

	public void MaskFill(SunGraphics2D sg2d,
			     SurfaceData sData,
			     Composite comp,
			     int x, int y, int w, int h,
			     byte mask[], int offset, int scan)
	{
	    BufferedImage dstBI =
		new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
	    SurfaceData tmpData = BufImgSurfaceData.createData(dstBI);

	    // REMIND: This is not pretty.  It would be nicer if we
	    // passed a "FillData" object to the Pixel loops, instead
	    // of a SunGraphics2D parameter...
	    int pixel = sg2d.pixel;
	    sg2d.pixel = tmpData.pixelFor(sg2d.getColor());
	    fillop.FillRect(sg2d, tmpData, 0, 0, w, h);
	    sg2d.pixel = pixel;

	    maskop.MaskBlit(tmpData, sData, comp, null,
			    0, 0, x, y, w, h,
			    mask, offset, scan);
	}
    }

    public GraphicsPrimitive traceWrap() {
	return new TraceMaskFill(this);
    }

    private static class TraceMaskFill extends MaskFill {
	MaskFill target;

	public TraceMaskFill(MaskFill target) {
	    super(target.getSourceType(),
		  target.getCompositeType(),
		  target.getDestType());
	    this.target = target;
	}

	public GraphicsPrimitive traceWrap() {
	    return this;
	}

	public void MaskFill(SunGraphics2D sg2d, SurfaceData sData,
			     Composite comp,
			     int x, int y, int w, int h,
			     byte[] mask, int maskoff, int maskscan)
	{
	    tracePrimitive(target);
	    target.MaskFill(sg2d, sData, comp, x, y, w, h,
			    mask, maskoff, maskscan);
	}
    }
}
