/*
 * @(#)SunCompositeContext.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.Composite;
import java.awt.CompositeContext;
import java.awt.AlphaComposite;
import java.awt.image.ColorModel;
import java.awt.image.BufferedImage;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import sun.awt.image.BufImgSurfaceData;
import sun.java2d.loops.XORComposite;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.Blit;

public class SunCompositeContext implements CompositeContext {
    ColorModel srcCM;
    ColorModel dstCM;
    Composite composite;
    CompositeType comptype;

    public SunCompositeContext(AlphaComposite ac,
			       ColorModel s, ColorModel d)
    {
        if (s == null) {
            throw new NullPointerException("Source color model cannot be null");
        }
        if (d == null) {
            throw new NullPointerException("Destination color model cannot be null");
        }
        srcCM = s;
        dstCM = d;
        this.composite = ac;
	this.comptype = CompositeType.forAlphaComposite(ac);
    }

    public SunCompositeContext(XORComposite xc,
			       ColorModel s, ColorModel d)
    {
        if (s == null) {
            throw new NullPointerException("Source color model cannot be null");
        }
        if (d == null) {
            throw new NullPointerException("Destination color model cannot be null");
        }
        srcCM = s;
        dstCM = d;
        this.composite = xc;
	this.comptype = CompositeType.Xor;
    }

    /**
     * Release resources allocated for context.
     */
    public void dispose() {
    }

    /**
     * This method composes the two source tiles
     * and places the result in the destination tile. Note that
     * the destination can be the same object as either
     * the first or second source.
     * @param src1 The first source tile for the compositing operation.
     * @param src2 The second source tile for the compositing operation.
     * @param dst The tile where the result of the operation is stored.
     */
    public void compose(Raster srcArg, Raster dstIn, WritableRaster dstOut) {
	WritableRaster src;
        int w;
        int h;

	if (dstIn != dstOut) {
	    dstOut.setDataElements(0, 0, dstIn);
	}

	// REMIND: We should be able to create a SurfaceData from just
	// a non-writable Raster and a ColorModel.  Since we need to
	// create a SurfaceData from a BufferedImage then we need to
	// make a WritableRaster since it is needed to construct a
	// BufferedImage.
	if (srcArg instanceof WritableRaster) {
	    src = (WritableRaster) srcArg;
	} else {
	    src = srcArg.createCompatibleWritableRaster();
	    src.setDataElements(0, 0, srcArg);
	}

	w = Math.min(src.getWidth(), dstIn.getWidth());
	h = Math.min(src.getHeight(), dstIn.getHeight());

	BufferedImage srcImg = new BufferedImage(srcCM, src,
						 srcCM.isAlphaPremultiplied(),
						 null);
	BufferedImage dstImg = new BufferedImage(dstCM, dstOut,
						 dstCM.isAlphaPremultiplied(),
						 null);

	SurfaceData srcData = BufImgSurfaceData.createData(srcImg);
	SurfaceData dstData = BufImgSurfaceData.createData(dstImg);
	Blit blit = Blit.getFromCache(srcData.getSurfaceType(),
				      comptype,
				      dstData.getSurfaceType());
	blit.Blit(srcData, dstData, composite, null, 0, 0, 0, 0, w, h);
    }
}
