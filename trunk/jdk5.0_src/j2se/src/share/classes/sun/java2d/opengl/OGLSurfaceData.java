/*
 * @(#)OGLSurfaceData.java	1.9 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.GraphicsEnvironment;
import java.awt.GradientPaint;
import java.awt.TexturePaint;
import java.awt.geom.AffineTransform;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import sun.awt.SunHints;
import sun.awt.image.PixelConverter;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.RenderLoops;
import sun.java2d.loops.SurfaceType;
import sun.java2d.pipe.PixelToShapeConverter;
import sun.java2d.pipe.TextPipe;

public abstract class OGLSurfaceData extends SurfaceData {

    /**
     * OGL-specific surface types
     */
    public static final int UNDEFINED       = 0;
    public static final int WINDOW          = 1;
    public static final int PIXMAP          = 2;
    public static final int PBUFFER         = 3;
    public static final int TEXTURE         = 4;
    public static final int VOL_BACKBUFFER  = 5;
    public static final int FLIP_BACKBUFFER = 6;

    /**
     * Pixel formats
     */
    public static final int PF_INT_ARGB        = 0;
    public static final int PF_INT_RGB         = 1;
    public static final int PF_INT_RGBX        = 2;
    public static final int PF_INT_BGR         = 3;
    public static final int PF_INT_BGRX        = 4;
    public static final int PF_3BYTE_BGR       = 5;
    public static final int PF_4BYTE_ABGR      = 6;
    public static final int PF_USHORT_565_RGB  = 7;
    public static final int PF_USHORT_555_RGB  = 8;
    public static final int PF_USHORT_555_RGBX = 9;
    public static final int PF_BYTE_GRAY       = 10;
    public static final int PF_USHORT_GRAY     = 11;
    public static final int PF_BYTE_INDEXED    = 12;
    public static final int PF_INDEX_8_GRAY    = 13;
    public static final int PF_INDEX_12_GRAY   = 14;
    public static final int PF_INT_ARGB_PRE    = 15;
    public static final int PF_4BYTE_ABGR_PRE  = 16;

    /**
     * SurfaceTypes
     */
    private static final String DESC_OPENGL_SURFACE = "OpenGL Surface";
    private static final String DESC_OPENGL_SURFACE_RTT =
        "OpenGL Surface (render-to-texture)";
    private static final String DESC_OPENGL_TEXTURE = "OpenGL Texture";

    public static final SurfaceType OpenGLSurface = 
	SurfaceType.Any.deriveSubType(DESC_OPENGL_SURFACE,
                                      PixelConverter.ArgbPre.instance);
    public static final SurfaceType OpenGLSurfaceRTT = 
	OpenGLSurface.deriveSubType(DESC_OPENGL_SURFACE_RTT);
    public static final SurfaceType OpenGLTexture = 
	SurfaceType.Any.deriveSubType(DESC_OPENGL_TEXTURE);

    protected RenderLoops solidloops;
    protected int type;
    protected int depth;

    protected static OGLRenderer
        oglSolidPipe, oglGradientPipe, oglTexturePipe;
    protected static PixelToShapeConverter
        oglTxSolidPipe, oglTxGradientPipe, oglTxTexturePipe;
    protected static OGLTextRenderer oglTextPipe;
    protected static OGLDrawImage oglImagePipe;

    private native void flush(long pData);
    protected native boolean initTexture(long pCtx, long pData,
                                         int width, int height);
    protected native boolean initFlipBackbuffer(long pData);
    protected native boolean initVolatileBackbuffer(long pData,
                                                    int width, int height);

    public abstract boolean initWindow();
    protected abstract boolean initPbuffer(long pCtx, long pData,
                                           int width, int height);
    protected abstract boolean initPixmap(long pCtx, long pData,
                                          int width, int height, int depth);
  
    static {
        if (!GraphicsEnvironment.isHeadless()) {
            oglSolidPipe = new OGLRenderer.Solid();
            oglGradientPipe = new OGLRenderer.Gradient();
            oglTexturePipe = new OGLRenderer.Texture();
            oglTextPipe = new OGLTextRenderer();
            if (GraphicsPrimitive.tracingEnabled()) {
                oglSolidPipe = oglSolidPipe.traceWrap();
                oglGradientPipe = oglGradientPipe.traceWrap();
                oglTexturePipe = oglTexturePipe.traceWrap();
                oglTextPipe = oglTextPipe.traceWrap();
            }
            oglTxSolidPipe = new PixelToShapeConverter(oglSolidPipe);
            oglTxGradientPipe = new PixelToShapeConverter(oglGradientPipe);
            oglTxTexturePipe = new PixelToShapeConverter(oglTexturePipe);
            oglImagePipe = new OGLDrawImage();

            OGLBlitLoops.register();
            OGLMaskFill.register();
            OGLMaskBlit.register();
        }
    }

    protected OGLSurfaceData(SurfaceType sType, ColorModel cm, int type) {
	super(sType, cm);
	this.depth = cm.getPixelSize();
        this.type = type;
    }

    /**
     * Initializes the appropriate OpenGL offscreen surface based on the value
     * of the type parameter.  If the surface creation fails for any reason,
     * an OutOfMemoryError will be thrown.
     */
    protected void initSurface(int width, int height, int depth) {
        boolean success = false;

        synchronized (OGLContext.LOCK) {
            long pData = getNativeOps();
            long pCtx;

            switch (type) {
            case PIXMAP:
                pCtx = getContext().getNativeContext();
                success = initPixmap(pCtx, pData, width, height, depth);
                break;

            case PBUFFER:
                pCtx = getContext().getNativeContext();
                success = initPbuffer(pCtx, pData, width, height);
                break;

            case TEXTURE:
                pCtx = OGLContext.getSharedContext(this);
                success = initTexture(pCtx, pData, width, height);
                break;

            case VOL_BACKBUFFER:
                if (initWindow()) {
                    success = initVolatileBackbuffer(pData, width, height);
                }
                break;

            case FLIP_BACKBUFFER:
                if (initWindow()) {
                    success = initFlipBackbuffer(pData);
                }
                break;
  
            default:
                break;
            }
        }

        if (!success) {
            throw new OutOfMemoryError("can't create offscreen surface");
        }
    }

    /**
     * Returns the OGLContext for the current thread and GraphicsConfig
     * associated with this surface.
     */
    public abstract OGLContext getContext();

    /**
     * Makes the native shared context current and returns a handle to the
     * native shared OGLContext structure.
     */
    public abstract long getSharedContext();

    public int getType() {
        return type;
    }

    public Raster getRaster(int x, int y, int w, int h) {
	throw new InternalError("not implemented yet");
    }

    public void validatePipe(SunGraphics2D sg2d) {
        if (sg2d.compositeState <= sg2d.COMP_XOR) {
            TextPipe textpipe;
            boolean validated = false;

            if ((sg2d.compositeState == sg2d.COMP_ISCOPY &&
                 sg2d.paintState == sg2d.PAINT_SOLIDCOLOR) ||
                (sg2d.compositeState == sg2d.COMP_ALPHA &&
                 sg2d.paintState <= sg2d.PAINT_SINGLECOLOR &&
                 (((AlphaComposite)sg2d.composite).getRule() ==
                  AlphaComposite.SRC_OVER)))
            {
                // OGLTextRenderer handles both AA and non-AA text, but
                // only works if composite is SrcNoEa or SrcOver
                textpipe = oglTextPipe;
            } else {
                // do this to initialize textpipe correctly; we will attempt
                // to override the non-text pipes below
                super.validatePipe(sg2d);
                textpipe = sg2d.textpipe;
                validated = true;
            }

            PixelToShapeConverter txPipe = null;
            OGLRenderer nonTxPipe = null;

            if (sg2d.antialiasHint != SunHints.INTVAL_ANTIALIAS_ON) {
                if (sg2d.paintState <= sg2d.PAINT_SINGLECOLOR) {
                    txPipe = oglTxSolidPipe;
                    nonTxPipe = oglSolidPipe;
                } else {
                    Class paintClass = sg2d.paint.getClass();
                    if (paintClass == GradientPaint.class) {
                        txPipe = oglTxGradientPipe;
                        nonTxPipe = oglGradientPipe;
                    } else if (paintClass == TexturePaint.class) {
                        TexturePaint paint = (TexturePaint)sg2d.paint;
                        if (OGLRenderer.Texture.isPaintValid(sg2d, paint)) {
                            txPipe = oglTxTexturePipe;
                            nonTxPipe = oglTexturePipe;
                        }
                    }
                    // custom paints handled by super.validatePipe() below
                }
            }

            if (txPipe != null) {
                if (sg2d.transformState >= sg2d.TRANSFORM_TRANSLATESCALE) {
                    sg2d.drawpipe = txPipe;
                    sg2d.fillpipe = txPipe;
                } else if (sg2d.strokeState != sg2d.STROKE_THIN) {
                    sg2d.drawpipe = txPipe;
                    sg2d.fillpipe = nonTxPipe;
                } else {
                    sg2d.drawpipe = nonTxPipe;
                    sg2d.fillpipe = nonTxPipe;
                }
                sg2d.shapepipe = nonTxPipe;
            } else {
                if (!validated) {
                    super.validatePipe(sg2d);
                }
            }

            // install the text pipe based on our earlier decision
            sg2d.textpipe = textpipe;
        } else {
            super.validatePipe(sg2d);
        }

        // always override the image pipe with the specialized OGL pipe
        sg2d.imagepipe = oglImagePipe;
    }

    public void lock() {
    }

    public void unlock() {
    }

    public RenderLoops getRenderLoops(SunGraphics2D sg2d) {
        if (sg2d.paintState == sg2d.PAINT_SOLIDCOLOR &&
            sg2d.compositeState == sg2d.COMP_ISCOPY)
        {
            return solidloops;
        }
        return super.getRenderLoops(sg2d);
    }

    public boolean copyArea(SunGraphics2D sg2d,
                            int x, int y, int w, int h, int dx, int dy)
    {
        CompositeType comptype = sg2d.imageComp;
        // REMIND: we could handle general transforms and xor (simply pass
        //         down the sg2d transform/xor color)...
        if (sg2d.transformState < sg2d.TRANSFORM_TRANSLATESCALE &&
            sg2d.compositeState < sg2d.COMP_XOR)
        {
            x += sg2d.transX;
            y += sg2d.transY;
            int dstx1 = x + dx;
            int dsty1 = y + dy;
            int dstx2 = dstx1 + w;
            int dsty2 = dsty1 + h;
            if (dstx1 < dstx2 && dsty1 < dsty2) {
                synchronized (OGLContext.LOCK) {
                    // REMIND: surface may not be opaque if it has stored
                    // alpha, but using SRC_IS_OPAQUE is safe for now since
                    // we don't currently support translucent VolatileImages
                    // in hardware...
                    int ctxflags =
                        OGLContext.SRC_IS_PREMULT |
                        OGLContext.SRC_IS_OPAQUE;
                    long oglc = OGLContext.getContext(this, this,
                                                      sg2d.getCompClip(),
                                                      sg2d.composite, null,
                                                      sg2d.pixel,
                                                      ctxflags);
                    oglSolidPipe.devCopyArea(oglc, getNativeOps(),
                                             dstx1 - dx, dsty1 - dy,
                                             dstx1, dsty1,
                                             dstx2 - dstx1, dsty2 - dsty1);
                }
            }
            return true;
        }
        return false;
    }

    public void flush() {
        synchronized (OGLContext.LOCK) {
            OGLContext.getSharedContext(this);
            flush(getNativeOps());
        }
    }

    /**
     * Returns true if non-premultiplied surfaces can have their alpha
     * values applied on-the-fly (using special OpenGL extensions)
     * during rendering operations to this surface.  If false, the caller
     * should use an alternate mechanism to ensure that the pixel values
     * sent to OpenGL are premultiplied.
     */
    public abstract boolean isBlendPremultAvailable();

    /**
     * Returns true if OpenGL textures can have non-power-of-two dimensions.
     */
    public abstract boolean isTexNonPow2Available();
}
