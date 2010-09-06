/*
 * @(#)ShapeDrawPipe.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.Shape;
import sun.java2d.SunGraphics2D;

/**
 * This interface defines the set of calls that pipeline objects
 * can use to pass on responsibility for drawing generic Shape
 * objects.
 */
public interface ShapeDrawPipe {
    public void draw(SunGraphics2D sg, Shape s);

    public void fill(SunGraphics2D sg, Shape s);
}
