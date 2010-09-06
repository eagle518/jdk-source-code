/*
 * @(#)LoopPipe.java	1.26 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.Font;
import java.awt.Shape;
import java.awt.BasicStroke;
import java.awt.Polygon;
import java.awt.geom.AffineTransform;
import java.awt.geom.PathIterator;
import java.awt.geom.RoundRectangle2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Arc2D;
import java.awt.geom.IllegalPathStateException;
import java.awt.font.GlyphVector;
import sun.dc.path.PathConsumer;
import sun.dc.path.PathException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.FontInfo;
import sun.java2d.loops.DrawPolygons;
import sun.awt.SunHints;

public class LoopPipe
    implements PixelDrawPipe,
	       PixelFillPipe,
	       ShapeDrawPipe
{
    public void drawLine(SunGraphics2D sg2d,
			 int x1, int y1, int x2, int y2)
    {
	int tX = sg2d.transX;
	int tY = sg2d.transY;
	sg2d.loops.drawLineLoop.DrawLine(sg2d, sg2d.getSurfaceData(),
					 x1 + tX, y1 + tY,
					 x2 + tX, y2 + tY);
    }

    public void drawRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	sg2d.loops.drawRectLoop.DrawRect(sg2d, sg2d.getSurfaceData(),
					 x + sg2d.transX,
					 y + sg2d.transY,
					 width, height);
    }

    public void drawRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	sg2d.shapepipe.draw(sg2d,
			    new RoundRectangle2D.Float(x, y, width, height,
						       arcWidth, arcHeight));
    }

    public void drawOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	sg2d.shapepipe.draw(sg2d, new Ellipse2D.Float(x, y, width, height));
    }

    public void drawArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	sg2d.shapepipe.draw(sg2d, new Arc2D.Float(x, y, width, height,
						  startAngle, arcAngle,
						  Arc2D.OPEN));
    }

    public void drawPolyline(SunGraphics2D sg2d,
			     int xPoints[], int yPoints[],
			     int nPoints)
    {
	int nPointsArray[] = { nPoints };
	sg2d.loops.drawPolygonsLoop.DrawPolygons(sg2d, sg2d.getSurfaceData(),
						 xPoints, yPoints,
						 nPointsArray, 1,
						 sg2d.transX, sg2d.transY,
						 false);
    }

    public void drawPolygon(SunGraphics2D sg2d,
			    int xPoints[], int yPoints[],
			    int nPoints)
    {
	int nPointsArray[] = { nPoints };
	sg2d.loops.drawPolygonsLoop.DrawPolygons(sg2d, sg2d.getSurfaceData(),
						 xPoints, yPoints,
						 nPointsArray, 1,
						 sg2d.transX, sg2d.transY,
						 true);
    }

    public void fillRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	sg2d.loops.fillRectLoop.FillRect(sg2d, sg2d.getSurfaceData(),
					 x + sg2d.transX,
					 y + sg2d.transY,
					 width, height);
    }

    public void fillRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	sg2d.shapepipe.fill(sg2d,
			    new RoundRectangle2D.Float(x, y, width, height,
						       arcWidth, arcHeight));
    }

    public void fillOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	sg2d.shapepipe.fill(sg2d, new Ellipse2D.Float(x, y, width, height));
    }

    public void fillArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	sg2d.shapepipe.fill(sg2d, new Arc2D.Float(x, y, width, height,
						  startAngle, arcAngle,
						  Arc2D.PIE));
    }

    public void fillPolygon(SunGraphics2D sg2d,
			    int xPoints[], int yPoints[],
			    int nPoints)
    {
	ShapeSpanIterator sr = new ShapeSpanIterator(sg2d, false);

	try {
	    sr.setOutputArea(sg2d.getCompClip());
	    sr.appendPoly(xPoints, yPoints, nPoints, sg2d.transX, sg2d.transY);
	    fillSpans(sg2d, sr);
	} finally {
	    sr.dispose();
	}
    }

    public void draw(SunGraphics2D sg2d, Shape s) {
	if (sg2d.strokeState == sg2d.STROKE_THIN) {
	    Polygon p = new Polygon();
	    AffineTransform transform =
		((sg2d.transformState >= sg2d.TRANSFORM_TRANSLATESCALE)
		 ? sg2d.transform
		 : null);
	    PathIterator pi = s.getPathIterator(transform, 0.5f);
	    float coords[] = new float[2];
	    int npoints[] = new int[1];
	    SurfaceData sd = sg2d.getSurfaceData();
	    DrawPolygons dp = sg2d.loops.drawPolygonsLoop;
	    while (!pi.isDone()) {
		switch (pi.currentSegment(coords)) {
		case PathIterator.SEG_MOVETO:
		    if (p.npoints > 1) {
			npoints[0] = p.npoints;
			dp.DrawPolygons(sg2d, sd,
					p.xpoints, p.ypoints, npoints, 1,
					sg2d.transX, sg2d.transY, false);
		    }
		    p.reset();
		    p.addPoint((int) Math.floor(coords[0]),
			       (int) Math.floor(coords[1]));
		    break;
		case PathIterator.SEG_LINETO:
		    if (p.npoints == 0) {
			throw new IllegalPathStateException
			    ("missing initial moveto in path definition");
		    }
		    p.addPoint((int) Math.floor(coords[0]),
			       (int) Math.floor(coords[1]));
		    break;
		case PathIterator.SEG_CLOSE:
		    if (p.npoints > 1) {
			npoints[0] = p.npoints;
			dp.DrawPolygons(sg2d, sd,
					p.xpoints, p.ypoints, npoints, 1,
					sg2d.transX, sg2d.transY, true);
			int x = p.xpoints[0];
			int y = p.ypoints[0];
			p.reset();
			p.addPoint(x, y);
		    }
		    break;
		default:
		    throw new IllegalPathStateException("path not flattened");
		}
		pi.next();
	    }
	    if (p.npoints > 1) {
		npoints[0] = p.npoints;
		dp.DrawPolygons(sg2d, sd,
				p.xpoints, p.ypoints, npoints, 1,
				sg2d.transX, sg2d.transY, false);
	    }
	    return;
	}

	if (sg2d.strokeState == sg2d.STROKE_CUSTOM) {
	    fill(sg2d, sg2d.stroke.createStrokedShape(s));
	    return;
	}

	ShapeSpanIterator sr = getStrokeSpans(sg2d, s);

	try {
	    fillSpans(sg2d, sr);
	} finally {
	    sr.dispose();
	}
    }

    /*
     * Return a ShapeSpanIterator to iterate the spans of the wide
     * outline of Shape s using the attributes of the SunGraphics2D
     * object.
     * REMIND: This should return a SpanIterator interface object
     * but the caller needs to dispose() the object and that method
     * is only on ShapeSpanIterator.
     * TODO: Add a dispose() method to the SpanIterator interface.
     */
    public static ShapeSpanIterator getStrokeSpans(SunGraphics2D sg2d,
						   Shape s)
    {
	ShapeSpanIterator sr = new ShapeSpanIterator(sg2d, true);

	try {
	    sr.setOutputArea(sg2d.getCompClip());
	    sr.setRule(PathIterator.WIND_NON_ZERO);

	    BasicStroke bs = (BasicStroke) sg2d.stroke;
	    AffineTransform transform =
		(sg2d.transformState >= sg2d.TRANSFORM_TRANSLATESCALE
		 ? sg2d.transform : null);
	    boolean thin = (sg2d.strokeState <= sg2d.STROKE_THINDASHED);

	    PathConsumer stroker =
		DuctusRenderer.createStroker(sr, bs, thin, transform);

	    try {
		transform =
		    ((sg2d.transformState == sg2d.TRANSFORM_ISIDENT)
		     ? null
		     : sg2d.transform);
		PathIterator pi = s.getPathIterator(transform);

		boolean adjust =
		    (sg2d.strokeHint != SunHints.INTVAL_STROKE_PURE);
		DuctusRenderer.feedConsumer(pi, stroker, adjust, 0.25f);
	    } catch (PathException e) {
		throw new InternalError("Unable to Stroke shape ("+
					e.getMessage()+")");
	    } finally {
		DuctusRenderer.disposeStroker(stroker, sr);
	    }
	} catch (Throwable t) {
	    sr.dispose();
	    sr = null;
	    throw new InternalError("Unable to Stroke shape ("+
				    t.getMessage()+")");
	}
	return sr;
    }

    public void fill(SunGraphics2D sg2d, Shape s) {
	ShapeSpanIterator sr = new ShapeSpanIterator(sg2d, false);
	try {
	    sr.setOutputArea(sg2d.getCompClip());
	    AffineTransform at =
		((sg2d.transformState == sg2d.TRANSFORM_ISIDENT)
		 ? null
		 : sg2d.transform);
	    sr.appendPath(s.getPathIterator(at));
	    fillSpans(sg2d, sr);
	} finally {
	    sr.dispose();
	}
    }

    private static void fillSpans(SunGraphics2D sg2d, SpanIterator si) {
	// REMIND: Eventually, the plan is that it will not be possible for
	// fs to be null since the FillSpans loop will be the fundamental
	// loop implemented for any destination type...
	if (sg2d.clipState == sg2d.CLIP_SHAPE) {
	    si = sg2d.clipRegion.filter(si);
	    // REMIND: Region.filter produces a Java-only iterator
	    // with no native counterpart...
	} else {
	    sun.java2d.loops.FillSpans fs = sg2d.loops.fillSpansLoop;
	    if (fs != null) {
		fs.FillSpans(sg2d, sg2d.getSurfaceData(), si);
		return;
	    }
	}
	int spanbox[] = new int[4];
	SurfaceData sd = sg2d.getSurfaceData();
	while (si.nextSpan(spanbox)) {
	    int x = spanbox[0];
	    int y = spanbox[1];
	    int w = spanbox[2] - x;
	    int h = spanbox[3] - y;
	    sg2d.loops.fillRectLoop.FillRect(sg2d, sd, x, y, w, h);
	}
    }
}
