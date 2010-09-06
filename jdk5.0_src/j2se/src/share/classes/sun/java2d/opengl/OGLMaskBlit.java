/*
 * @(#)OGLMaskBlit.java	1.6 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.Composite;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.Blit;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.MaskBlit;
import sun.java2d.loops.SurfaceType;
import sun.java2d.pipe.Region;

/**
 * The MaskBlit operation is expressed as:
 *   dst = ((src <MODE> dst) * pathA) + (dst * (1 - pathA))
 *
 * The OGL implementation of the MaskBlit operation differs from the above
 * equation because it is not possible to perform such a complex operation in
 * OpenGL (without the use of advanced techniques like fragment shaders and
 * multitexturing).  Therefore, the OGLMaskBlit operation is expressed as:
 *   dst = (src * pathA) <SrcOver> dst
 *
 * This simplified formula is only equivalent to the "true" MaskBlit equation
 * in the following situations:
 *   - <MODE> is SrcOver
 *   - <MODE> is Src, extra alpha == 1.0, and the source surface is opaque
 *
 * Therefore, we register OGLMaskBlit primitives for only the SurfaceType and
 * CompositeType restrictions mentioned above.  In addition for the Src
 * case, we must override the composite with a SrcOver (no extra alpha)
 * instance, so that we set up the OpenGL blending mode to match the
 * OGLMaskBlit equation.
 */
public class OGLMaskBlit extends MaskBlit {

    public static void register() {
        GraphicsPrimitive[] primitives = {
            new OGLMaskBlit(SurfaceType.IntArgb,
                            CompositeType.SrcOver,
                            OGLSurfaceData.PF_INT_ARGB),
            new OGLMaskBlit(SurfaceType.IntRgb,
                            CompositeType.SrcOver,
                            OGLSurfaceData.PF_INT_RGB),
            new OGLMaskBlit(SurfaceType.IntRgb,
                            CompositeType.SrcNoEa,
                            OGLSurfaceData.PF_INT_RGB),
            new OGLMaskBlit(SurfaceType.IntBgr,
                            CompositeType.SrcOver,
                            OGLSurfaceData.PF_INT_BGR),
            new OGLMaskBlit(SurfaceType.IntBgr,
                            CompositeType.SrcNoEa,
                            OGLSurfaceData.PF_INT_BGR),
        };
        GraphicsPrimitiveMgr.register(primitives);
    }

    private int typeval;
    private Blit blitop;

    public OGLMaskBlit(SurfaceType srcType,
                       CompositeType compType,
                       int typeval)
    {
        super(srcType, compType, OGLSurfaceData.OpenGLSurface);
        this.typeval = typeval;
    }

    private native void MaskBlit(long pCtx, long pSrcOps,
                                 int srcx, int srcy,
                                 int dstx, int dsty,
                                 int width, int height,
                                 byte[] mask, int maskoff, int maskscan,
                                 int srctype);

    public void MaskBlit(SurfaceData src, SurfaceData dst,
                         Composite comp, Region clip,
                         int srcx, int srcy,
                         int dstx, int dsty,
                         int width, int height,
                         byte[] mask, int maskoff, int maskscan)
    {
        if (mask == null) {
            // no mask involved; delegate to regular blit loop
            if (blitop == null) {
                blitop = Blit.getFromCache(src.getSurfaceType(),
                                           CompositeType.AnyAlpha,
                                           OGLSurfaceData.OpenGLSurface);
            }
            blitop.Blit(src, dst,
                        comp, clip,
                        srcx, srcy, dstx, dsty,
                        width, height);
            return;
        }

        AlphaComposite acomp = (AlphaComposite)comp;
        if (acomp.getRule() != AlphaComposite.SRC_OVER) {
            comp = AlphaComposite.SrcOver;
        }

        synchronized (OGLContext.LOCK) {
            OGLSurfaceData oglDst = (OGLSurfaceData)dst;
            long pSrcOps = src.getNativeOps();
            long pCtx = OGLContext.getContext(oglDst, oglDst,
                                              clip, comp, null, 0,
                                              OGLContext.SRC_IS_PREMULT);

            MaskBlit(pCtx, pSrcOps,
                     srcx, srcy, dstx, dsty,
                     width, height,
                     mask, maskoff, maskscan,
                     typeval);
        }
    }
}
