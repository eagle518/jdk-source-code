/*
 * @(#)X11Renderer.java	1.22 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.Composite;
import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.PathIterator;
import java.awt.geom.IllegalPathStateException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.XORComposite;
import sun.java2d.pipe.Region;
import sun.java2d.pipe.PixelDrawPipe;
import sun.java2d.pipe.PixelFillPipe;
import sun.java2d.pipe.ShapeDrawPipe;
import sun.java2d.pipe.SpanIterator;
import sun.java2d.pipe.ShapeSpanIterator;
import sun.java2d.pipe.LoopPipe;

public class X11Renderer implements
    PixelDrawPipe,
    PixelFillPipe,
    ShapeDrawPipe
{
    private static final Object AWT_LOCK = X11GraphicsEnvironment.class;

    private static native long XCreateGC(long pXSData);
    private static native void XSetClip(long xgc,
					int lox, int loy, int hix, int hiy,
					Region complexclip);
    private static native void XSetCopyMode(long xgc);
    private static native void XSetXorMode(long xgc);
    private static native void XSetForeground(long xgc, int pixel);

    private long xgc;

    private Region validatedClip;
    private XORComposite validatedXorComp;
    private int xorpixelmod;
    private int validatedPixel;

    public static X11Renderer getInstance(X11SurfaceData xsd) {
	return (GraphicsPrimitive.tracingEnabled()
		? new X11TracingRenderer(xsd)
		: new X11Renderer(xsd));
    }

    protected X11Renderer(X11SurfaceData xsd) {
	long pXSData = xsd.getNativeOps();
	synchronized (AWT_LOCK) {
	    xgc = XCreateGC(pXSData);
	}
    }

    private final void validate(SunGraphics2D sg2d) {
	// assert: AWT_LOCK is synchronized;

	// NOTE: getCompClip() will revalidateAll() if the
	// surfaceData is invalid.  This should ensure that
	// the clip and pixel that we are validating against
	// are the most current. 
	//
	// The assumption is that the pipeline after that
	// revalidation will either be another X11 pipe
	// (because the drawable format never changes on X11)
	// or a null pipeline if the surface is disposed.
	//
	// Since we do not get the ops structure of the SurfaceData
	// until the actual call down to the native level we will
	// pick up the most recently validated copy.
	// Note that if the surface is disposed, a NullSurfacData
	// (with null native data structure) will be set in
	// sg2d, so we have to protect against it in native code.

	Region clip = sg2d.getCompClip();
	if (clip != validatedClip) {
	    validatedClip = clip;
	    XSetClip(xgc,
		     clip.getLoX(), clip.getLoY(),
		     clip.getHiX(), clip.getHiY(),
		     (clip.isRectangular() ? null : clip));
	}
	if (sg2d.compositeState == sg2d.COMP_ISCOPY) {
	    if (validatedXorComp != null) {
		validatedXorComp = null;
		xorpixelmod = 0;
		XSetCopyMode(xgc);
	    }
	} else {
	    if (validatedXorComp != sg2d.composite) {
		validatedXorComp = (XORComposite) sg2d.composite;
		xorpixelmod = validatedXorComp.getXorPixel();
		XSetXorMode(xgc);
	    }
	}
	int pixel = sg2d.pixel ^ xorpixelmod;
	if (pixel != validatedPixel) {
	    validatedPixel = pixel;
	    XSetForeground(xgc, pixel);
	}
    }

    native void XDrawLine(long pXSData, long xgc,
			  int x1, int y1, int x2, int y2);

    public void drawLine(SunGraphics2D sg2d, int x1, int y1, int x2, int y2) {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    int transx = sg2d.transX;
	    int transy = sg2d.transY;
	    XDrawLine(sg2d.surfaceData.getNativeOps(), xgc,
		      x1+transx, y1+transy, x2+transx, y2+transy);
	}
    }

    native void XDrawRect(long pXSData, long xgc,
			  int x, int y, int w, int h);

    public void drawRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawRect(sg2d.surfaceData.getNativeOps(), xgc,
		      x+sg2d.transX, y+sg2d.transY, width, height);
	}
    }

    native void XDrawRoundRect(long pXSData, long xgc,
			       int x, int y, int w, int h,
			       int arcW, int arcH);

    public void drawRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawRoundRect(sg2d.surfaceData.getNativeOps(), xgc,
			   x+sg2d.transX, y+sg2d.transY, width, height,
			   arcWidth, arcHeight);
	}
    }

    native void XDrawOval(long pXSData, long xgc,
			  int x, int y, int w, int h);

    public void drawOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawOval(sg2d.surfaceData.getNativeOps(), xgc,
		      x+sg2d.transX, y+sg2d.transY, width, height);
	}
    }

    native void XDrawArc(long pXSData, long xgc,
			 int x, int y, int w, int h,
			 int angleStart, int angleExtent);

    public void drawArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawArc(sg2d.surfaceData.getNativeOps(), xgc,
		     x+sg2d.transX, y+sg2d.transY, width, height,
		     startAngle, arcAngle);
	}
    }

    native void XDrawPoly(long pXSData, long xgc,
			  int transx, int transy,
			  int[] xpoints, int[] ypoints,
			  int npoints, boolean isclosed);

    public void drawPolyline(SunGraphics2D sg2d,
			     int xpoints[], int ypoints[],
			     int npoints)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawPoly(sg2d.surfaceData.getNativeOps(), xgc,
		      sg2d.transX, sg2d.transY,
		      xpoints, ypoints, npoints, false);
	}
    }

    public void drawPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XDrawPoly(sg2d.surfaceData.getNativeOps(), xgc,
		      sg2d.transX, sg2d.transY,
		      xpoints, ypoints, npoints, true);
	}
    }

    native void XFillRect(long pXSData, long xgc,
			  int x, int y, int w, int h);

    public void fillRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XFillRect(sg2d.surfaceData.getNativeOps(), xgc,
		      x+sg2d.transX, y+sg2d.transY, width, height);
	}
    }

    native void XFillRoundRect(long pXSData, long xgc,
			       int x, int y, int w, int h,
			       int arcW, int arcH);

    public void fillRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XFillRoundRect(sg2d.surfaceData.getNativeOps(), xgc,
			   x+sg2d.transX, y+sg2d.transY, width, height,
			   arcWidth, arcHeight);
	}
    }

    native void XFillOval(long pXSData, long xgc,
			  int x, int y, int w, int h);

    public void fillOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XFillOval(sg2d.surfaceData.getNativeOps(), xgc,
		      x+sg2d.transX, y+sg2d.transY, width, height);
	}
    }

    native void XFillArc(long pXSData, long xgc,
			 int x, int y, int w, int h,
			 int angleStart, int angleExtent);

    public void fillArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XFillArc(sg2d.surfaceData.getNativeOps(), xgc,
		     x+sg2d.transX, y+sg2d.transY, width, height,
		     startAngle, arcAngle);
	}
    }

    native void XFillPoly(long pXSData, long xgc,
			  int transx, int transy,
			  int[] xpoints, int[] ypoints,
			  int npoints);

    public void fillPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	synchronized (AWT_LOCK) {
	    validate(sg2d);
	    XFillPoly(sg2d.surfaceData.getNativeOps(), xgc,
		      sg2d.transX, sg2d.transY, xpoints, ypoints, npoints);
	}
    }

    native void XFillSpans(long pXSData, long xgc,
			   SpanIterator si, long iterator,
			   int transx, int transy);

    public void draw(SunGraphics2D sg2d, Shape s) {
	if (sg2d.strokeState == sg2d.STROKE_THIN) {
	    AffineTransform at;
	    Polygon p;
	    if (sg2d.transformState < sg2d.TRANSFORM_TRANSLATESCALE) {
		if (s instanceof Polygon) {
		    p = (Polygon) s;
		    drawPolygon(sg2d, p.xpoints, p.ypoints, p.npoints);
		    return;
		}
		at = null;
	    } else {
		at = sg2d.transform;
	    }
	    PathIterator pi = s.getPathIterator(at, 0.5f);
	    p = new Polygon();
	    float coords[] = new float[2];
	    while (!pi.isDone()) {
		switch (pi.currentSegment(coords)) {
		case PathIterator.SEG_MOVETO:
		    if (p.npoints > 1) {
			drawPolyline(sg2d, p.xpoints, p.ypoints, p.npoints);
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
		    if (p.npoints > 0) {
			p.addPoint(p.xpoints[0], p.ypoints[0]);
		    }
		    break;
		default:
		    throw new IllegalPathStateException("path not flattened");
		}
		pi.next();
	    }
	    if (p.npoints > 1) {
		drawPolyline(sg2d, p.xpoints, p.ypoints, p.npoints);
	    }
	} else if (sg2d.strokeState < sg2d.STROKE_CUSTOM) {
	    // REMIND: X11 can handle uniform scaled wide lines
	    // and dashed lines itself if we set the appropriate
	    // XGC attributes (TBD).
	    ShapeSpanIterator si = LoopPipe.getStrokeSpans(sg2d, s);
	    try {
		synchronized (AWT_LOCK) {
		    validate(sg2d);
		    XFillSpans(sg2d.surfaceData.getNativeOps(), xgc,
			       si, si.getNativeIterator(),
			       0, 0);
		}
	    } finally {
		si.dispose();
	    }
	} else {
	    fill(sg2d, sg2d.stroke.createStrokedShape(s));
	}
    }

    public void fill(SunGraphics2D sg2d, Shape s) {
	AffineTransform at;
	int transx, transy;

	if (sg2d.transformState < sg2d.TRANSFORM_TRANSLATESCALE) {
	    if (s instanceof Polygon) {
		Polygon p = (Polygon) s;
		fillPolygon(sg2d, p.xpoints, p.ypoints, p.npoints);
		return;
	    }
	    // Transform (translation) will be done by XFillSpans
	    at = null;
	    transx = sg2d.transX;
	    transy = sg2d.transY;
	} else {
	    // Transform will be done by the PathIterator
	    at = sg2d.transform;
	    transx = transy = 0;
	}
	ShapeSpanIterator ssi = new ShapeSpanIterator(sg2d, false);
	try {
	    // Subtract transx/y from the SSI clip to match the
	    // (potentially untranslated) geometry fed to it
	    Region clip = sg2d.getCompClip();
	    ssi.setOutputAreaXYXY(clip.getLoX() - transx,
				  clip.getLoY() - transy,
				  clip.getHiX() - transx,
				  clip.getHiY() - transy);
	    ssi.appendPath(s.getPathIterator(at));
	    synchronized (AWT_LOCK) {
		validate(sg2d);
		XFillSpans(sg2d.surfaceData.getNativeOps(), xgc,
			   ssi, ssi.getNativeIterator(),
			   transx, transy);
	    }
	} finally {
	    ssi.dispose();
	}
    }

    native void devCopyArea(SurfaceData sData,
			    int srcx, int srcy,
			    int dstx, int dsty,
			    int w, int h);

    public static class X11TracingRenderer extends X11Renderer {
	public X11TracingRenderer(X11SurfaceData xsd) {
	    super(xsd);
	}

	void XDrawLine(long pXSData, long xgc,
		       int x1, int y1, int x2, int y2)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawLine");
	    super.XDrawLine(pXSData, xgc, x1, y1, x2, y2);
	}
	void XDrawRect(long pXSData, long xgc,
		       int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawRect");
	    super.XDrawRect(pXSData, xgc, x, y, w, h);
	}
	void XDrawRoundRect(long pXSData, long xgc,
			    int x, int y, int w, int h,
			    int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawRoundRect");
	    super.XDrawRoundRect(pXSData, xgc, x, y, w, h, arcW, arcH);
	}
	void XDrawOval(long pXSData, long xgc,
		       int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawOval");
	    super.XDrawOval(pXSData, xgc, x, y, w, h);
	}
	void XDrawArc(long pXSData, long xgc,
		      int x, int y, int w, int h,
		      int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawArc");
	    super.XDrawArc(pXSData, xgc,
			   x, y, w, h, angleStart, angleExtent);
	}
	void XDrawPoly(long pXSData, long xgc,
		       int transx, int transy,
		       int[] xpoints, int[] ypoints,
		       int npoints, boolean isclosed)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawPoly");
	    super.XDrawPoly(pXSData, xgc, transx, transy,
			    xpoints, ypoints, npoints, isclosed);
	}
	void XFillRect(long pXSData, long xgc,
		       int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillRect");
	    super.XFillRect(pXSData, xgc, x, y, w, h);
	}
	void XFillRoundRect(long pXSData, long xgc,
			    int x, int y, int w, int h,
			    int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillRoundRect");
	    super.XFillRoundRect(pXSData, xgc, x, y, w, h, arcW, arcH);
	}
	void XFillOval(long pXSData, long xgc,
		       int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillOval");
	    super.XFillOval(pXSData, xgc, x, y, w, h);
	}
	void XFillArc(long pXSData, long xgc,
		      int x, int y, int w, int h,
		      int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillArc");
	    super.XFillArc(pXSData, xgc,
			   x, y, w, h, angleStart, angleExtent);
	}
	void XFillPoly(long pXSData, long xgc,
		       int transx, int transy,
		       int[] xpoints, int[] ypoints,
		       int npoints)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillPoly");
	    super.XFillPoly(pXSData, xgc,
			    transx, transy, xpoints, ypoints, npoints);
	}
	void XFillSpans(long pXSData, long xgc,
			SpanIterator si, long iterator, int transx, int transy)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillSpans");
	    super.XFillSpans(pXSData, xgc,
			     si, iterator, transx, transy);
	}
	void devCopyArea(SurfaceData sData,
			 int srcx, int srcy,
			 int dstx, int dsty,
			 int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11CopyArea");
	    super.devCopyArea(sData, srcx, srcy, dstx, dsty, w, h);
	}
    }
}
