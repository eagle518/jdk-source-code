/*
 * @(#)RenderLoops.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.loops;

/*
 * This class stores the various loops that are used by the
 * standard rendering pipelines.  The loops for a given instance
 * of this class will all share the same destination type and the
 * same supported paint and composite operation.
 * Each instance of this class should be shared by all graphics
 * objects that render onto the same type of destination with the
 * same paint and composite combination to reduce the amount of
 * time spent looking up loops appropriate for the current fill
 * technique.
 */
public class RenderLoops {

    public static final int primTypeID = GraphicsPrimitive.makePrimTypeID(); 

    public DrawLine		drawLineLoop;
    public FillRect		fillRectLoop;
    public DrawRect		drawRectLoop;
    public DrawPolygons		drawPolygonsLoop;
    public FillSpans		fillSpansLoop;
    public DrawGlyphList	drawGlyphListLoop;
    public DrawGlyphListAA	drawGlyphListAALoop;
}
