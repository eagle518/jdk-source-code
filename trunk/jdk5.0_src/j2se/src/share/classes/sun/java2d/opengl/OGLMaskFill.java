/*
 * @(#)OGLMaskFill.java	1.5 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.Composite;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.MaskFill;
import sun.java2d.pipe.Region;

/**
 * The MaskFill operation is expressed as:
 *   dst = ((src <MODE> dst) * pathA) + (dst * (1 - pathA))
 *
 * The OGL implementation of the MaskFill operation differs from the above
 * equation because it is not possible to perform such a complex operation in
 * OpenGL (without the use of advanced techniques like fragment shaders and
 * multitexturing).  Therefore, the OGLMaskFill operation is expressed as:
 *   dst = (src * pathA) <SrcOver> dst
 *
 * This simplified formula is only equivalent to the "true" MaskFill equation
 * in the following situations:
 *   - <MODE> is SrcOver
 *   - <MODE> is Src, extra alpha == 1.0, and the source color is opaque
 *
 * Therefore, we register OGLMaskFill primitives for only the SurfaceType and
 * CompositeType restrictions mentioned above.  In addition for the Src
 * case, we must override the composite with a SrcOver (no extra alpha)
 * instance, so that we set up the OpenGL blending mode to match the
 * OGLMaskFill equation.
 */
public class OGLMaskFill extends MaskFill {

    public static void register() {
        GraphicsPrimitive[] primitives = {
            new OGLMaskFill(SurfaceType.AnyColor, CompositeType.SrcOver),
            new OGLMaskFill(SurfaceType.OpaqueColor, CompositeType.SrcNoEa),
	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    public OGLMaskFill(SurfaceType srcType, CompositeType compType) {
        super(srcType, compType, OGLSurfaceData.OpenGLSurface);
    }

    private native void MaskFill(long pCtx,
                                 int x, int y, int w, int h,
                                 byte[] mask, int maskoff, int maskscan);

    public void MaskFill(SunGraphics2D sg2d, SurfaceData sData,
                         Composite comp,
                         int x, int y, int w, int h,
                         byte[] mask, int maskoff, int maskscan)
    {
        AlphaComposite acomp = (AlphaComposite)comp;
        if (acomp.getRule() != AlphaComposite.SRC_OVER) {
            comp = AlphaComposite.SrcOver;
        }

        synchronized (OGLContext.LOCK) {
            OGLSurfaceData oglDst = (OGLSurfaceData)sData;
            long pCtx = OGLContext.getContext(oglDst, oglDst,
                                              sg2d.getCompClip(), comp,
                                              null, sg2d.pixel,
                                              OGLContext.SRC_IS_PREMULT);

            MaskFill(pCtx, x, y, w, h, mask, maskoff, maskscan);
        }
    }
}
