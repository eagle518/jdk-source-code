/*
 * @(#)Win32Renderer.java	1.12 04/01/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Composite;
import java.awt.Shape;
import java.awt.geom.GeneralPath;
import java.awt.geom.PathIterator;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.pipe.Region;
import sun.java2d.pipe.PixelDrawPipe;
import sun.java2d.pipe.PixelFillPipe;
import sun.java2d.pipe.ShapeDrawPipe;
import sun.java2d.pipe.SpanIterator;
import sun.java2d.pipe.ShapeSpanIterator;
import sun.java2d.pipe.LoopPipe;
import sun.java2d.loops.GraphicsPrimitive;

public class Win32Renderer implements
    PixelDrawPipe,
    PixelFillPipe,
    ShapeDrawPipe
{
    private static native void initIDs();

    static {
	initIDs();
    }

    native void doDrawLine(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int x1, int y1, int x2, int y2);

    public void drawLine(SunGraphics2D sg2d,
			 int x1, int y1, int x2, int y2)
    {
	int transx = sg2d.transX;
	int transy = sg2d.transY;
	doDrawLine(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   x1+transx, y1+transy, x2+transx, y2+transy);
    }

    native void doDrawRect(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int x, int y, int w, int h);

    public void drawRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doDrawRect(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doDrawRoundRect(SurfaceData sData,
				Region clip, Composite comp, int color,
				int x, int y, int w, int h,
				int arcW, int arcH);

    public void drawRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	doDrawRoundRect(sg2d.surfaceData,
                        sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
			x+sg2d.transX, y+sg2d.transY, width, height,
			arcWidth, arcHeight);
    }

    native void doDrawOval(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int x, int y, int w, int h);

    public void drawOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doDrawOval(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doDrawArc(SurfaceData sData,
			  Region clip, Composite comp, int color,
			  int x, int y, int w, int h,
			  int angleStart, int angleExtent);

    public void drawArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	doDrawArc(sg2d.surfaceData,
                  sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		  x+sg2d.transX, y+sg2d.transY, width, height,
		  startAngle, arcAngle);
    }

    native void doDrawPoly(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int transx, int transy,
			   int[] xpoints, int[] ypoints,
			   int npoints, boolean isclosed);

    public void drawPolyline(SunGraphics2D sg2d,
			     int xpoints[], int ypoints[],
			     int npoints)
    {
	doDrawPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints, false);
    }

    public void drawPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	doDrawPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints, true);
    }

    native void doFillRect(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int x, int y, int w, int h);

    public void fillRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doFillRect(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doFillRoundRect(SurfaceData sData,
				Region clip, Composite comp, int color,
				int x, int y, int w, int h,
				int arcW, int arcH);

    public void fillRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	doFillRoundRect(sg2d.surfaceData,
                        sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
			x+sg2d.transX, y+sg2d.transY, width, height,
			arcWidth, arcHeight);
    }

    native void doFillOval(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int x, int y, int w, int h);

    public void fillOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doFillOval(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doFillArc(SurfaceData sData,
			  Region clip, Composite comp, int color,
			  int x, int y, int w, int h,
			  int angleStart, int angleExtent);

    public void fillArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	doFillArc(sg2d.surfaceData,
                  sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		  x+sg2d.transX, y+sg2d.transY, width, height,
		  startAngle, arcAngle);
    }

    native void doFillPoly(SurfaceData sData,
			   Region clip, Composite comp, int color,
			   int transx, int transy,
			   int[] xpoints, int[] ypoints,
			   int npoints);

    public void fillPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	doFillPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints);
    }

    native void doShape(SurfaceData sData,
			Region clip, Composite comp, int color,
			int transX, int transY,
			GeneralPath gp, boolean isfill);

    void doShape(SunGraphics2D sg2d, Shape s, boolean isfill) {
	GeneralPath gp;
	int transX;
	int transY;
	if (s instanceof GeneralPath) {
	    if (sg2d.transformState >= sg2d.TRANSFORM_TRANSLATESCALE) {
		s = ((GeneralPath)s).createTransformedShape(sg2d.transform);
		transX = 0;
		transY = 0;
	    } else {
		transX = sg2d.transX;
		transY = sg2d.transY;
	    } 
	    gp = (GeneralPath) s;
	} else {
	    PathIterator pi = s.getPathIterator(sg2d.transform);
	    gp = new GeneralPath(pi.getWindingRule());
	    gp.append(pi, false);
	    transX = 0;
	    transY = 0;
	}
	doShape(sg2d.surfaceData,
                sg2d.getCompClip(), sg2d.composite, sg2d.rgb,
		transX, transY, gp, isfill);
    }

    // REMIND: This is just a hack to get WIDE lines to honor the
    // necessary hinted pixelization rules.  This should be replaced
    // by a native FillSpans method or a getHintedStrokeGeneralPath()
    // method that could be filled by the doShape method more quickly.
    public void doFillSpans(SunGraphics2D sg2d, SpanIterator si) {
	int box[] = new int[4];
	SurfaceData sd = sg2d.surfaceData;
	Region clip = sg2d.getCompClip();
        Composite comp = sg2d.composite;
	int rgb = sg2d.rgb;
	while (si.nextSpan(box)) {
	    doFillRect(sd, clip, comp, rgb,
		       box[0], box[1], box[2]-box[0], box[3]-box[1]);
	}
    }

    public void draw(SunGraphics2D sg2d, Shape s) {
	if (sg2d.strokeState == sg2d.STROKE_THIN) {
	    doShape(sg2d, s, false);
	} else if (sg2d.strokeState < sg2d.STROKE_CUSTOM) {
	    ShapeSpanIterator si = LoopPipe.getStrokeSpans(sg2d, s);
	    try {
		doFillSpans(sg2d, si);
	    } finally {
		si.dispose();
	    }
	} else {
	    doShape(sg2d, sg2d.stroke.createStrokedShape(s), true);
	}
    }

    public void fill(SunGraphics2D sg2d, Shape s) {
	doShape(sg2d, s, true);
    }

    native void devCopyArea(SurfaceData sData,
			    int srcx, int srcy,
			    int dx, int dy,
			    int w, int h);

    public Win32Renderer traceWrap() {
	return new Tracer();
    }

    public static class Tracer extends Win32Renderer {
	void doDrawLine(SurfaceData sData,
			Region clip, Composite comp, int color,
			int x1, int y1, int x2, int y2)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawLine");
	    super.doDrawLine(sData, clip, comp, color, x1, y1, x2, y2);
	}
	void doDrawRect(SurfaceData sData,
			Region clip, Composite comp, int color,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawRect");
	    super.doDrawRect(sData, clip, comp, color, x, y, w, h);
	}
	void doDrawRoundRect(SurfaceData sData,
			     Region clip, Composite comp, int color,
			     int x, int y, int w, int h,
			     int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawRoundRect");
	    super.doDrawRoundRect(sData, clip, comp, color,
                                  x, y, w, h, arcW, arcH);
	}
	void doDrawOval(SurfaceData sData,
			Region clip, Composite comp, int color,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawOval");
	    super.doDrawOval(sData, clip, comp, color, x, y, w, h);
	}
	void doDrawArc(SurfaceData sData,
		       Region clip, Composite comp, int color,
		       int x, int y, int w, int h,
		       int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawArc");
	    super.doDrawArc(sData, clip, comp, color, x, y, w, h,
			    angleStart, angleExtent);
	}
	void doDrawPoly(SurfaceData sData,
			Region clip, Composite comp, int color,
			int transx, int transy,
			int[] xpoints, int[] ypoints,
			int npoints, boolean isclosed)
	{
	    GraphicsPrimitive.tracePrimitive("GDIDrawPoly");
	    super.doDrawPoly(sData, clip, comp, color, transx, transy,
			     xpoints, ypoints, npoints, isclosed);
	}
	void doFillRect(SurfaceData sData,
			Region clip, Composite comp, int color,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("GDIFillRect");
	    super.doFillRect(sData, clip, comp, color, x, y, w, h);
	}
	void doFillRoundRect(SurfaceData sData,
			     Region clip, Composite comp, int color,
			     int x, int y, int w, int h,
			     int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("GDIFillRoundRect");
	    super.doFillRoundRect(sData, clip, comp, color,
                                  x, y, w, h, arcW, arcH);
	}
	void doFillOval(SurfaceData sData,
			Region clip, Composite comp, int color,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("GDIFillOval");
	    super.doFillOval(sData, clip, comp, color, x, y, w, h);
	}
	void doFillArc(SurfaceData sData,
		       Region clip, Composite comp, int color,
		       int x, int y, int w, int h,
		       int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("GDIFillArc");
	    super.doFillArc(sData, clip, comp, color, x, y, w, h,
			    angleStart, angleExtent);
	}
	void doFillPoly(SurfaceData sData,
			Region clip, Composite comp, int color,
			int transx, int transy,
			int[] xpoints, int[] ypoints,
			int npoints)
	{
	    GraphicsPrimitive.tracePrimitive("GDIFillPoly");
	    super.doFillPoly(sData, clip, comp, color, transx, transy,
			     xpoints, ypoints, npoints);
	}
	void doShape(SurfaceData sData,
		     Region clip, Composite comp, int color,
		     int transX, int transY,
		     GeneralPath gp, boolean isfill)
	{
	    GraphicsPrimitive.tracePrimitive(isfill
					     ? "GDIFillShape"
					     : "GDIDrawShape");
	    super.doShape(sData, clip, comp, color,
                          transX, transY, gp, isfill);
	}
	void devCopyArea(SurfaceData sData,
			 int srcx, int srcy,
			 int dx, int dy,
			 int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("GDICopyArea");
	    super.devCopyArea(sData, srcx, srcy, dx, dy, w, h);
	}
    }
}
