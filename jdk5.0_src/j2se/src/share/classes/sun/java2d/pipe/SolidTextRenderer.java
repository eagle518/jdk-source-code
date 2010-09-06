/*
 * @(#)SolidTextRenderer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.font.GlyphVector;
import sun.java2d.SunGraphics2D;
import sun.font.GlyphList;

/**
 * A delegate pipe of SG2D for drawing text with 
 * a solid source colour to an opaque destination.
 */

public class SolidTextRenderer extends GlyphListPipe {

   /*
    * Override super class method to call the AA pipe if 
    * AA is specified in the GlyphVector's FontRenderContext
    */
   public void drawGlyphVector(SunGraphics2D sg2d, GlyphVector g,
                               float x, float y) {

        if (g.getFontRenderContext().isAntiAliased()) {
            sg2d.surfaceData.aaTextRenderer.drawGlyphVector(sg2d, g, x, y);
        } else {
            super.drawGlyphVector(sg2d, g, x, y);
        }
   }

   protected void drawGlyphList(SunGraphics2D sg2d, GlyphList gl) {
       sg2d.loops.drawGlyphListLoop.DrawGlyphList(sg2d, sg2d.surfaceData, gl);
   }
}
