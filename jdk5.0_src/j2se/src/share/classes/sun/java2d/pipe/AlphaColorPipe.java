/*
 * @(#)AlphaColorPipe.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.Rectangle;
import java.awt.Shape;
import sun.java2d.SunGraphics2D;

/**
 * This class implements a CompositePipe that renders path alpha tiles
 * into a destination that supports direct alpha compositing of a solid
 * color, according to one of the rules in the AlphaComposite class.
 */
public class AlphaColorPipe implements CompositePipe {
    public AlphaColorPipe() {
    }

    public Object startSequence(SunGraphics2D sg, Shape s, Rectangle dev,
				int[] abox) {
	return sg;
    }

    public boolean needTile(Object context, int x, int y, int w, int h) {
	return true;
    }

    public void renderPathTile(Object context,
			       byte[] atile, int offset, int tilesize,
			       int x, int y, int w, int h) {
	SunGraphics2D sg = (SunGraphics2D) context;

	sg.alphafill.MaskFill(sg, sg.getSurfaceData(), sg.composite,
			      x, y, w, h,
			      atile, offset, tilesize);
    }

    public void skipTile(Object context, int x, int y) {
	return;
    }

    public void endSequence(Object context) {
	return;
    }
}
