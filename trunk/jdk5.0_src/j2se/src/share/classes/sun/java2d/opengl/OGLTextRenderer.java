/*
 * @(#)OGLTextRenderer.java	1.6 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.Composite;
import java.awt.font.GlyphVector;
import sun.font.GlyphList;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.pipe.GlyphListPipe;
import sun.java2d.pipe.Region;

/**
 * The native OGLTextRenderer implementation is similar to the OGLMaskFill
 * operation and shares the same restrictions regarding SurfaceType and
 * CompositeType.  See OGLMaskFill.java for more information.
 */
public class OGLTextRenderer extends GlyphListPipe {

    native void doDrawGlyphList(long pCtx,
                                Region clip, GlyphList gl);

    protected void drawGlyphList(SunGraphics2D sg2d, GlyphList gl) {
        AlphaComposite comp = (AlphaComposite)sg2d.composite;
        if (comp.getRule() != AlphaComposite.SRC_OVER) {
            comp = AlphaComposite.SrcOver;
        }

        synchronized (OGLContext.LOCK) {
            OGLSurfaceData dstData = (OGLSurfaceData)sg2d.surfaceData;
            long pCtx = OGLContext.getContext(dstData, dstData,
                                              sg2d.getCompClip(), comp,
                                              null, sg2d.pixel,
                                              OGLContext.SRC_IS_PREMULT);

            doDrawGlyphList(pCtx, sg2d.getCompClip(), gl);
        }
    }

    public OGLTextRenderer traceWrap() {
        return new Tracer();
    }

    public static class Tracer extends OGLTextRenderer {
	void doDrawGlyphList(long pCtx,
                             Region clip, GlyphList gl)
        {
	    GraphicsPrimitive.tracePrimitive("OGLDrawGlyphs");
	    super.doDrawGlyphList(pCtx, clip, gl);
	}
    }
}
