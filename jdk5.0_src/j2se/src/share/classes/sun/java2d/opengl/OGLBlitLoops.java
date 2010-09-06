/*
 * @(#)OGLBlitLoops.java	1.7 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Transparency;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import sun.awt.image.BufImgSurfaceData;
import sun.java2d.SurfaceData;
import sun.java2d.loops.Blit;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.ScaledBlit;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.TransformBlit;
import sun.java2d.pipe.Region;

public class OGLBlitLoops {

    public static void register() {
        Blit blitIntArgbPreToSurface =
            new OGLSwToSurfaceBlit(SurfaceType.IntArgbPre,
                                   OGLSurfaceData.PF_INT_ARGB_PRE);
        Blit blitIntArgbPreToTexture =
            new OGLSwToTextureBlit(SurfaceType.IntArgbPre,
                                   OGLSurfaceData.PF_INT_ARGB_PRE);

        // REMIND: with some changes to OGLDrawImage, we could handle
        //         all scale ops via TransformBlit, so we'd only have to
        //         register TransformBlits... same for BlitBg...
        GraphicsPrimitive[] primitives = {
            // surface->surface ops
            new OGLSurfaceToSurfaceBlit(),
            new OGLSurfaceToSurfaceScale(),
            new OGLSurfaceToSurfaceTransform(),

            // render-to-texture surface->surface ops
            new OGLRTTSurfaceToSurfaceBlit(),
            new OGLRTTSurfaceToSurfaceScale(),
            new OGLRTTSurfaceToSurfaceTransform(),

            // surface->sw ops
            new OGLSurfaceToSwBlit(SurfaceType.IntArgb,
                                   OGLSurfaceData.PF_INT_ARGB),

            // sw->surface ops
            blitIntArgbPreToSurface,
            new OGLSwToSurfaceBlit(SurfaceType.IntRgb,
                                   OGLSurfaceData.PF_INT_RGB),
            new OGLSwToSurfaceBlit(SurfaceType.IntRgbx,
                                   OGLSurfaceData.PF_INT_RGBX),
            new OGLSwToSurfaceBlit(SurfaceType.IntBgr,
                                   OGLSurfaceData.PF_INT_BGR),
            new OGLSwToSurfaceBlit(SurfaceType.IntBgrx,
                                   OGLSurfaceData.PF_INT_BGRX),
            new OGLSwToSurfaceBlit(SurfaceType.Ushort565Rgb,
                                   OGLSurfaceData.PF_USHORT_565_RGB),
            new OGLSwToSurfaceBlit(SurfaceType.Ushort555Rgb,
                                   OGLSurfaceData.PF_USHORT_555_RGB),
            new OGLSwToSurfaceBlit(SurfaceType.Ushort555Rgbx,
                                   OGLSurfaceData.PF_USHORT_555_RGBX),
            new OGLSwToSurfaceBlit(SurfaceType.ByteGray,
                                   OGLSurfaceData.PF_BYTE_GRAY),
            new OGLSwToSurfaceBlit(SurfaceType.UshortGray,
                                   OGLSurfaceData.PF_USHORT_GRAY),
            new OGLDelegateBlit(
                SurfaceType.IntArgb,
                CompositeType.AnyAlpha,
                OGLSurfaceData.OpenGLSurface,
                new OGLSwToSurfaceBlit(SurfaceType.IntArgb,
                                       OGLSurfaceData.PF_INT_ARGB),
                new OGLGeneralBlit(OGLSurfaceData.OpenGLSurface,
                                   CompositeType.AnyAlpha,
                                   blitIntArgbPreToSurface)),

            new OGLSwToSurfaceScale(SurfaceType.IntRgb,
                                    OGLSurfaceData.PF_INT_RGB),
            new OGLSwToSurfaceScale(SurfaceType.IntRgbx,
                                    OGLSurfaceData.PF_INT_RGBX),
            new OGLSwToSurfaceScale(SurfaceType.IntBgr,
                                    OGLSurfaceData.PF_INT_BGR),
            new OGLSwToSurfaceScale(SurfaceType.IntBgrx,
                                    OGLSurfaceData.PF_INT_BGRX),
            new OGLSwToSurfaceScale(SurfaceType.Ushort565Rgb,
                                    OGLSurfaceData.PF_USHORT_565_RGB),
            new OGLSwToSurfaceScale(SurfaceType.Ushort555Rgb,
                                    OGLSurfaceData.PF_USHORT_555_RGB),
            new OGLSwToSurfaceScale(SurfaceType.Ushort555Rgbx,
                                    OGLSurfaceData.PF_USHORT_555_RGBX),
            new OGLSwToSurfaceScale(SurfaceType.ByteGray,
                                    OGLSurfaceData.PF_BYTE_GRAY),
            new OGLSwToSurfaceScale(SurfaceType.UshortGray,
                                    OGLSurfaceData.PF_USHORT_GRAY),
            new OGLSwToSurfaceScale(SurfaceType.IntArgbPre,
                                    OGLSurfaceData.PF_INT_ARGB_PRE),
            // REMIND: need delegate/general scale loops?
            //new OGLSwToSurfaceScale(SurfaceType.IntArgb,
            //                        OGLSurfaceData.PF_INT_ARGB),

            new OGLSwToSurfaceTransform(SurfaceType.IntRgb,
                                        OGLSurfaceData.PF_INT_RGB),
            new OGLSwToSurfaceTransform(SurfaceType.IntRgbx,
                                        OGLSurfaceData.PF_INT_RGBX),
            new OGLSwToSurfaceTransform(SurfaceType.IntBgr,
                                        OGLSurfaceData.PF_INT_BGR),
            new OGLSwToSurfaceTransform(SurfaceType.IntBgrx,
                                        OGLSurfaceData.PF_INT_BGRX),
            new OGLSwToSurfaceTransform(SurfaceType.Ushort565Rgb,
                                        OGLSurfaceData.PF_USHORT_565_RGB),
            new OGLSwToSurfaceTransform(SurfaceType.Ushort555Rgb,
                                        OGLSurfaceData.PF_USHORT_555_RGB),
            new OGLSwToSurfaceTransform(SurfaceType.Ushort555Rgbx,
                                        OGLSurfaceData.PF_USHORT_555_RGBX),
            new OGLSwToSurfaceTransform(SurfaceType.ByteGray,
                                        OGLSurfaceData.PF_BYTE_GRAY),
            new OGLSwToSurfaceTransform(SurfaceType.UshortGray,
                                        OGLSurfaceData.PF_USHORT_GRAY),
            new OGLSwToSurfaceTransform(SurfaceType.IntArgbPre,
                                        OGLSurfaceData.PF_INT_ARGB_PRE),
            // REMIND: need delegate/general transform loops?
            //new OGLSwToSurfaceTransform(SurfaceType.IntArgb,
            //                            OGLSurfaceData.PF_INT_ARGB),

            // texture->surface ops
            new OGLTextureToSurfaceBlit(),
            new OGLTextureToSurfaceScale(),
            new OGLTextureToSurfaceTransform(),

            // texture->sw ops
            new OGLTextureToSwBlit(),

            // sw->texture ops
            blitIntArgbPreToTexture,
            new OGLSwToTextureBlit(SurfaceType.IntRgb,
                                   OGLSurfaceData.PF_INT_RGB),
            new OGLSwToTextureBlit(SurfaceType.IntRgbx,
                                   OGLSurfaceData.PF_INT_RGBX),
            new OGLSwToTextureBlit(SurfaceType.IntBgr,
                                   OGLSurfaceData.PF_INT_BGR),
            new OGLSwToTextureBlit(SurfaceType.IntBgrx,
                                   OGLSurfaceData.PF_INT_BGRX),
            new OGLSwToTextureBlit(SurfaceType.Ushort565Rgb,
                                   OGLSurfaceData.PF_USHORT_565_RGB),
            new OGLSwToTextureBlit(SurfaceType.Ushort555Rgb,
                                   OGLSurfaceData.PF_USHORT_555_RGB),
            new OGLSwToTextureBlit(SurfaceType.Ushort555Rgbx,
                                   OGLSurfaceData.PF_USHORT_555_RGBX),
            new OGLSwToTextureBlit(SurfaceType.ByteGray,
                                   OGLSurfaceData.PF_BYTE_GRAY),
            new OGLSwToTextureBlit(SurfaceType.UshortGray,
                                   OGLSurfaceData.PF_USHORT_GRAY),
            new OGLDelegateBlit(
                SurfaceType.IntArgb,
                CompositeType.SrcNoEa,
                OGLSurfaceData.OpenGLTexture,
                new OGLSwToTextureBlit(SurfaceType.IntArgb,
                                       OGLSurfaceData.PF_INT_ARGB),
                new OGLGeneralBlit(OGLSurfaceData.OpenGLTexture,
                                   CompositeType.SrcNoEa,
                                   blitIntArgbPreToTexture)),
	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    private static native void Blit(long pCtx, long pSrcOps, long pDstOps,
                                    AffineTransform xform, int hint,
                                    int srcx, int srcy, int dstx, int dsty,
                                    int srcw, int srch, int dstw, int dsth,
                                    int srctype, boolean texture);

    public static void Blit(SurfaceData srcData, SurfaceData dstData,
                            Composite comp, Region clip,
                            AffineTransform xform, int hint,
                            int srcx, int srcy, int dstx, int dsty,
                            int srcw, int srch, int dstw, int dsth,
                            int srctype, boolean texture)
    {
        // set each component of the fragment color to the extra alpha
        // value, which will effectively apply the extra alpha to each
        // pixel in the texture (this is only applicable when this is a
        // sw->texture->surface copy operation)
        int pixel = 0xffffffff;

        ColorModel cm = srcData.getColorModel();
        boolean opaque = (cm.getTransparency() == Transparency.OPAQUE);

        int ctxflags = 0;
        if (cm.isAlphaPremultiplied() || opaque) {
            // we always treat opaque sources as premultiplied
            ctxflags |= OGLContext.SRC_IS_PREMULT;
        }
        if (opaque) {
            ctxflags |= OGLContext.SRC_IS_OPAQUE;
        }
        if (!texture) {
            ctxflags |= OGLContext.USE_EXTRA_ALPHA;
        }

        synchronized (OGLContext.LOCK) {
            OGLSurfaceData oglDst = (OGLSurfaceData)dstData;
            long pSrcOps = srcData.getNativeOps();
            long pDstOps = dstData.getNativeOps();
            long pCtx;

            if (texture) {
                pCtx = OGLContext.getSharedContext(oglDst);
            } else {
                pCtx = OGLContext.getContext(oglDst, oglDst,
                                             clip, comp, xform, pixel,
                                             ctxflags);
            }

            Blit(pCtx, pSrcOps, pDstOps,
                 xform, hint,
                 srcx, srcy, dstx, dsty,
                 srcw, srch, dstw, dsth,
                 srctype, texture);
        }
    }

    private static native void IsoBlit(long pCtx, long pSrcOps, long pDstOps,
                                       AffineTransform xform, int hint,
                                       int srcx, int srcy, int dstx, int dsty,
                                       int srcw, int srch, int dstw, int dsth,
                                       boolean texture, boolean rtt);

    public static void IsoBlit(SurfaceData srcData, SurfaceData dstData,
                               Composite comp, Region clip,
                               AffineTransform xform, int hint,
                               int srcx, int srcy, int dstx, int dsty,
                               int srcw, int srch, int dstw, int dsth,
                               boolean texture)
    {
        // set each component of the fragment color to the extra alpha value,
        // which will effectively apply the extra alpha to each pixel in the
        // texture (this is applicable when this is a texture->surface or
        // surface->texture->surface copy operation)
        int pixel = 0xffffffff;

        int ctxflags = 0;
        if (!texture) {
            // REMIND: surface may not be opaque if it has stored alpha,
            // but this is safe for now since we don't currently support
            // translucent VolatileImages in hardware...
            ctxflags |= OGLContext.SRC_IS_OPAQUE;
            if (xform == null) {
                // we specify USE_EXTRA_ALPHA here so that we use
                // glPixelTransfer() to apply the extra alpha value as part
                // of the glCopyPixels() operation (when there is a transform,
                // the USE_EXTRA_ALPHA flag is not appropriate because the
                // extra alpha will be applied separately as part of the
                // surface->texture->surface copy operation)
                ctxflags |= OGLContext.USE_EXTRA_ALPHA;
            }
        }
        ColorModel cm = srcData.getColorModel();
        if (cm.getTransparency() == Transparency.OPAQUE ||
            cm.isAlphaPremultiplied())
        {
            // we always treat opaque sources as premultiplied
            ctxflags |= OGLContext.SRC_IS_PREMULT;
        }

        synchronized (OGLContext.LOCK) {
            OGLSurfaceData oglSrc = (OGLSurfaceData)srcData;
            OGLSurfaceData oglDst = (OGLSurfaceData)dstData;
            long pSrcOps = srcData.getNativeOps();
            long pDstOps = dstData.getNativeOps();

            boolean rtt;
            OGLSurfaceData srcCtxData;
            if (oglSrc.getType() == OGLSurfaceData.TEXTURE) {
                // the source is a regular texture object; we substitute
                // the destination surface for the purposes of making a
                // context current
                rtt = false;
                srcCtxData = oglDst;
            } else {
                // the source is a render-to-texture surface; we always
                // treat these as opaque, so modify the context flags
                // accordingly
                ctxflags |= OGLContext.SRC_IS_OPAQUE;
                rtt = true;
                srcCtxData = oglSrc;
            }

            long pCtx = OGLContext.getContext(srcCtxData, oglDst,
                                              clip, comp, xform, pixel,
                                              ctxflags);

            IsoBlit(pCtx, pSrcOps, pDstOps,
                    xform, hint,
                    srcx, srcy, dstx, dsty,
                    srcw, srch, dstw, dsth,
                    texture, rtt);
        }
    }

    public static native void SurfaceToSwBlit(long pCtx,
                                              long pSrcOps, long pDstOps,
                                              int srcx, int srcy,
                                              int dstx, int dsty,
                                              int w, int h, int dsttype);
}

class OGLSurfaceToSurfaceBlit extends Blit {

    public OGLSurfaceToSurfaceBlit() {
        super(OGLSurfaceData.OpenGLSurface,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null,
                             AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             false);
    }
}

class OGLSurfaceToSurfaceScale extends ScaledBlit {
    
    public OGLSurfaceToSurfaceScale() {
        super(OGLSurfaceData.OpenGLSurface,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }
    
    public void Scale(SurfaceData src, SurfaceData dst,
                      Composite comp, Region clip,
                      int sx, int sy, int dx, int dy,
                      int sw, int sh, int dw, int dh)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null,
                             AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                             sx, sy, dx, dy,
                             sw, sh, dw, dh,
                             false);
    }
}

class OGLSurfaceToSurfaceTransform extends TransformBlit {

    public OGLSurfaceToSurfaceTransform() {
        super(OGLSurfaceData.OpenGLSurface,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Transform(SurfaceData src, SurfaceData dst, 
                          Composite comp, Region clip,
                          AffineTransform at, int hint,
                          int sx, int sy, int dx, int dy,
                          int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, at, hint,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             false);
    }
}

class OGLRTTSurfaceToSurfaceBlit extends Blit {

    public OGLRTTSurfaceToSurfaceBlit() {
        super(OGLSurfaceData.OpenGLSurfaceRTT,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null,
                             AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             true);
    }
}

class OGLRTTSurfaceToSurfaceScale extends ScaledBlit {

    public OGLRTTSurfaceToSurfaceScale() {
        super(OGLSurfaceData.OpenGLSurfaceRTT,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Scale(SurfaceData src, SurfaceData dst,
                      Composite comp, Region clip,
                      int sx, int sy, int dx, int dy,
                      int sw, int sh, int dw, int dh)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null,
                             AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                             sx, sy, dx, dy,
                             sw, sh, dw, dh,
                             true);
    }
}

class OGLRTTSurfaceToSurfaceTransform extends TransformBlit {

    public OGLRTTSurfaceToSurfaceTransform() {
        super(OGLSurfaceData.OpenGLSurfaceRTT,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Transform(SurfaceData src, SurfaceData dst,
                          Composite comp, Region clip,
                          AffineTransform at, int hint,
                          int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, at, hint,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             true);
    }
}

class OGLSurfaceToSwBlit extends Blit {

    private int typeval;

    // REMIND: destination will actually be opaque/premultiplied...
    public OGLSurfaceToSwBlit(SurfaceType dstType, int typeval) {
        super(OGLSurfaceData.OpenGLSurface,
              CompositeType.SrcNoEa,
              dstType);
        this.typeval = typeval;
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy,
                     int w, int h)
    {
        synchronized (OGLContext.LOCK) {
            OGLSurfaceData oglSrc = (OGLSurfaceData)src;
            long pSrcOps = src.getNativeOps();
            long pDstOps = dst.getNativeOps();
            long pCtx = OGLContext.getContext(oglSrc);
            OGLBlitLoops.SurfaceToSwBlit(pCtx, pSrcOps, pDstOps,
                                         sx, sy, dx, dy,
                                         w, h, typeval);
        }
    }
}

class OGLSwToSurfaceBlit extends Blit {

    private int typeval;

    public OGLSwToSurfaceBlit(SurfaceType srcType, int typeval) {
        super(srcType,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
        this.typeval = typeval;
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.Blit(src, dst,
                          comp, clip, null,
                          AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                          sx, sy, dx, dy,
                          w, h, w, h,
                          typeval, false);
    }
}

class OGLSwToSurfaceScale extends ScaledBlit {
    
    private int typeval;

    public OGLSwToSurfaceScale(SurfaceType srcType, int typeval) {
        super(srcType,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
        this.typeval = typeval;
    }
    
    public void Scale(SurfaceData src, SurfaceData dst,
                      Composite comp, Region clip,
                      int sx, int sy, int dx, int dy,
                      int sw, int sh, int dw, int dh)
    {
        OGLBlitLoops.Blit(src, dst,
                          comp, clip, null,
                          AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                          sx, sy, dx, dy,
                          sw, sh, dw, dh,
                          typeval, false);
    }
}

class OGLSwToSurfaceTransform extends TransformBlit {

    private int typeval;

    public OGLSwToSurfaceTransform(SurfaceType srcType, int typeval) {
        super(srcType,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
        this.typeval = typeval;
    }

    public void Transform(SurfaceData src, SurfaceData dst,
                          Composite comp, Region clip,
                          AffineTransform at, int hint,
                          int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.Blit(src, dst,
                          comp, clip, at, hint,
                          sx, sy, dx, dy,
                          w, h, w, h,
                          typeval, false);
    }
}

class OGLSwToTextureBlit extends Blit {

    private int typeval;

    public OGLSwToTextureBlit(SurfaceType srcType, int typeval) {
        super(srcType,
              CompositeType.SrcNoEa,
              OGLSurfaceData.OpenGLTexture);
        this.typeval = typeval;
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.Blit(src, dst,
                          comp, clip, null,
                          AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                          sx, sy, dx, dy,
                          w, h, w, h,
                          typeval, true);
    }
}

class OGLTextureToSurfaceBlit extends Blit {

    public OGLTextureToSurfaceBlit() {
        super(OGLSurfaceData.OpenGLTexture,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null,
                             AffineTransformOp.TYPE_NEAREST_NEIGHBOR,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             true);
    }
}

class OGLTextureToSurfaceScale extends ScaledBlit {

    public OGLTextureToSurfaceScale() {
        super(OGLSurfaceData.OpenGLTexture,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }
    
    public void Scale(SurfaceData src, SurfaceData dst,
                      Composite comp, Region clip,
                      int sx, int sy, int dx, int dy,
                      int sw, int sh, int dw, int dh)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, null, 0,
                             sx, sy, dx, dy,
                             sw, sh, dw, dh,
                             true);
    }
}

class OGLTextureToSurfaceTransform extends TransformBlit {

    public OGLTextureToSurfaceTransform() {
        super(OGLSurfaceData.OpenGLTexture,
              CompositeType.AnyAlpha,
              OGLSurfaceData.OpenGLSurface);
    }

    public void Transform(SurfaceData src, SurfaceData dst, 
                          Composite comp, Region clip,
                          AffineTransform at, int hint,
                          int sx, int sy, int dx, int dy,
                          int w, int h)
    {
        OGLBlitLoops.IsoBlit(src, dst,
                             comp, clip, at, hint,
                             sx, sy, dx, dy,
                             w, h, w, h,
                             true);
    }
}

class OGLTextureToSwBlit extends Blit {

    public OGLTextureToSwBlit() {
        super(OGLSurfaceData.OpenGLTexture,
              CompositeType.SrcNoEa,
              SurfaceType.IntArgb);
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy,
                     int w, int h)
    {
        // REMIND: if this method is invoked, it's because there is no direct
        //         primitive registered to get the source surface down to
        //         the texture, so we need to go through a convert blit,
        //         which requires fetching the "texture raster", which doesn't
        //         really exist or have useful data in it... so for now
        //         we'll assume dst is already appropriately cleared, and
        //         we'll treat this as a no-op...
    }
}

/**
 * This general Blit implemenation converts any source surface to an
 * intermediate IntArgbPre surface, and then uses the more specific
 * IntArgbPre->OpenGLSurface/Texture loop to get the intermediate
 * (premultiplied) surface down to OpenGL.
 */
class OGLGeneralBlit extends Blit {

    private Blit performop;

    public OGLGeneralBlit(SurfaceType dstType,
                          CompositeType compType,
                          Blit performop)
    {
        super(SurfaceType.Any, compType, dstType);
        this.performop = performop;
    }

    public void Blit(SurfaceData src, SurfaceData dst,
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        SurfaceData srcTmp;
        Blit convertsrc = Blit.getFromCache(src.getSurfaceType(),
                                            CompositeType.SrcNoEa,
                                            SurfaceType.IntArgbPre);

        // create intermediate IntArgbPre surface
        BufferedImage srcTmpBI =
            new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB_PRE);
        srcTmp = BufImgSurfaceData.createData(srcTmpBI);

        // convert source to IntArgbPre
	convertsrc.Blit(src, srcTmp, AlphaComposite.Src, null,
                        sx, sy, 0, 0, w, h);

        // copy IntArgbPre intermediate surface to OpenGL surface
        performop.Blit(srcTmp, dst, comp, clip,
                       0, 0, dx, dy, w, h);
    }
}

/**
 * This is a special Blit primitive that is constructed with two Blit
 * choices: one for the case when the "blendPremult" extension is available,
 * and the other primitive for when that extension is not available.  So if
 * blendPremult is available, we can simply let OpenGL take care of
 * non-premultiplied surfaces (e.g. IntArgb).  If blendPremult is not
 * available, we will delegate to the "premultNotAvailBlit" to ensure that
 * non-premultiplied surfaces are converted to IntArgbPre before
 * sending them down to OpenGL.
 */
class OGLDelegateBlit extends Blit {

    private Blit premultAvailBlit;
    private Blit premultNotAvailBlit;

    public OGLDelegateBlit(SurfaceType srcType,
                           CompositeType compType,
                           SurfaceType dstType,
                           Blit premultAvailBlit,
                           Blit premultNotAvailBlit)
    {
        super(srcType, compType, dstType);
        this.premultAvailBlit = premultAvailBlit;
        this.premultNotAvailBlit = premultNotAvailBlit;
    }

    public void Blit(SurfaceData src, SurfaceData dst, 
                     Composite comp, Region clip,
                     int sx, int sy, int dx, int dy, int w, int h)
    {
        Blit op;

        if (((OGLSurfaceData)dst).isBlendPremultAvailable()) {
            op = premultAvailBlit;
        } else {
            op = premultNotAvailBlit;
        }

        op.Blit(src, dst, comp, clip, sx, sy, dx, dy, w, h);
    }
}
