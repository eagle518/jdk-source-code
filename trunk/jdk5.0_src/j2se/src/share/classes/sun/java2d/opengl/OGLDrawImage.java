/*
 * @(#)OGLDrawImage.java	1.7 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Color;
import java.awt.Image;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.TransformBlit;
import sun.java2d.pipe.DrawImage;

public class OGLDrawImage extends DrawImage {
    protected void renderImageXform(SunGraphics2D sg, Image img,
				    AffineTransform tx, int interpType,
				    int sx1, int sy1, int sx2, int sy2,
				    Color bgColor)
    {
        // punt to the MediaLib-based transformImage() in the superclass if:
        //     - bicubic interpolation is specified
        //     - a background color is specified and will be used
        //     - the source surface is not a texture and a non-default
        //       interpolation hint is specified (we can only control the
        //       filtering for texture->surface copies)
        //         REMIND: we can tweak the sw->texture->surface
        //         transform case to handle filtering appropriately...
        //     - an appropriate TransformBlit primitive could not be found
        if (interpType != AffineTransformOp.TYPE_BICUBIC) {
	    SurfaceData sData =
		SurfaceData.getSourceSurfaceData(img, sg.surfaceData,
						 sg.imageComp, bgColor, true);

	    if (sData != null &&
		!isBgOperation(sData, bgColor) &&
		(sData.getSurfaceType() == OGLSurfaceData.OpenGLTexture ||
		 interpType == AffineTransformOp.TYPE_NEAREST_NEIGHBOR))
	    {
		SurfaceType srcType = sData.getSurfaceType();
		SurfaceType dstType = sg.surfaceData.getSurfaceType();
		TransformBlit blit = TransformBlit.getFromCache(srcType,
								sg.imageComp,
								dstType);

		if (blit != null) {
		    blit.Transform(sData, sg.surfaceData,
				   sg.composite, sg.getCompClip(),
				   tx, interpType,
				   sx1, sy1, 0, 0, sx2-sx1, sy2-sy1);
		    return;
		}
	    }
	}

	super.renderImageXform(sg, img, tx, interpType,
			       sx1, sy1, sx2, sy2, bgColor);
    }
}
