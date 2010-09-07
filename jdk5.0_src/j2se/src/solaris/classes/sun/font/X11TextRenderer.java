/*
 * @(#)X11TextRenderer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.font;

import java.awt.Rectangle;
import java.awt.font.GlyphVector;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.pipe.GlyphListPipe;
import sun.java2d.pipe.Region;
import sun.java2d.loops.GraphicsPrimitive;

/**
 * A delegate pipe of SG2D for drawing text with 
 * a solid source colour to an X11 drawable destination.
 */
public class X11TextRenderer extends GlyphListPipe {
    /*
     * Override super class method to call the AA pipe if 
     * AA is specified in the GlyphVector's FontRenderContext
     */
    public void drawGlyphVector(SunGraphics2D sg2d, GlyphVector g,
				float x, float y)
    {
        if (g.getFontRenderContext().isAntiAliased()) {
            sg2d.surfaceData.aaTextRenderer.drawGlyphVector(sg2d, g, x, y);
        } else {
            super.drawGlyphVector(sg2d, g, x, y);
        }
    }

    native void doDrawGlyphList(SurfaceData sd,
				Region clip, int pixel,
				GlyphList gl);

    protected void drawGlyphList(SunGraphics2D sg2d, GlyphList gl) {
	doDrawGlyphList(sg2d.surfaceData, sg2d.getCompClip(), sg2d.pixel, gl);
    }

    public X11TextRenderer traceWrap() {
	return new Tracer();
    }

    public static class Tracer extends X11TextRenderer {
	void doDrawGlyphList(SurfaceData sd,
			     Region clip, int pixel,
			     GlyphList gl)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawGlyphs");
	    super.doDrawGlyphList(sd, clip, pixel, gl);
	}
    }
}
