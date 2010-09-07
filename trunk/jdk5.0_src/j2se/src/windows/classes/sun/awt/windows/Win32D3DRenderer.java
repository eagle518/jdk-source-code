/*
 * @(#)Win32D3DRenderer.java	1.7 03/12/19
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
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GeneralRenderer;

/**
 * Win32D3DRenderer
 * 
 * This class accelerates rendering to a surface by using Direct3D rendering 
 * when possible.
 * The renderers in here are simply java wrappers around native methods that
 * do the real work.
 */
public class Win32D3DRenderer extends Win32DDRenderer
{
    boolean canHandleClipping = false;
    
    public Win32D3DRenderer(boolean canHandleClipping) {
	this.canHandleClipping = canHandleClipping;
    }
    
    //
    // Native implementations
    //

    /**
     * Native methods to rendering lines and rects.  A return value
     * of true indicates success using Direct3D, whereas
     * false indicates failure; after failure, the caller should
     * both handle the primitive through other means and disable
     * 3D rendering on the surfaceData.
     */
    native boolean doDrawLineD3D(SurfaceData sData,
				 int color,
				 int x1, int y1, int x2, int y2,
				 int clipX1, int clipY1, int clipX2, int clipY2);
    native boolean doDrawRectD3D(SurfaceData sData,
				 int color,
				 int x, int y, int w, int h,
				 int clipX1, int clipY1, int clipX2, int clipY2);

    //
    // Internal Java methods for rendering
    //

    /**
     * Checks whether the line can be trivially accepted.
     * A return value of false does not necessarily mean that
     * that the line will be clipped, but that a more rigorous
     * test is needed.
     */
    private boolean lineClipNeeded(Region clip, 
				   int x1, int y1, int x2, int y2)
    {
    	// If any of these are true, the line lies outside of the
	// clip bounds	
	int cx1 = clip.getLoX();
	int cy1 = clip.getLoY();
	int cx2 = clip.getHiX();
	int cy2 = clip.getHiY();
	return (x1 < cx1 || x2 < cx1 ||
		x1 >= cx2 || x2 >= cx2 ||
		y1 < cy1 || y2 < cy1 ||
		y1 >= cy2 || y2 >= cy2);
    }

    /**
     * Checks whether the rectangle can be trivially accepted.
     * A return value of false does not necessarily mean that
     * that the rect will be clipped, but that a more rigorous
     * test is needed.
     */
    private boolean rectClipNeeded(Region clip,
				   int x, int y, int width, int height)
    {
	return !clip.encompassesXYWH(x, y, width+1, height+1);
    }


    // REMIND: It would be nice to do fillSpans to issue large
    // batches of d3d lines...
        
    //
    // Java wrappers for the primitive renderers
    //

    /**
     * drawLine draws a line between the pixel at x1, y1 and the
     * pixel at x2, y2 (including the last pixel).
     */
    public void drawLine(SunGraphics2D sg2d,
			 int x1, int y1, int x2, int y2)
    {
	// DDraw handles horizontal/vertical lines faster than d3d,
	// so punt to DDRenderer in these cases
	if (x1 == x2 || y1 == y2) {
	    super.drawLine(sg2d, x1, y1, x2, y2);
	    return;
	}
	int transx1 = x1 + sg2d.transX;
	int transy1 = y1 + sg2d.transY;		
	int transx2 = x2 + sg2d.transX;
	int transy2 = y2 + sg2d.transY;
	Region clip = sg2d.getCompClip();
	if (!canHandleClipping &&
	    lineClipNeeded(clip, transx1, transy1, transx2, transy2)) 
	{
	    // Punt to our superclass renderer to render clipped lines
	    super.drawLine(sg2d, x1, y1, x2, y2);
	    return;
	}
	if (!doDrawLineD3D(sg2d.surfaceData, sg2d.rgb, 
			   transx1, transy1,
			   transx2, transy2,
			   clip.getLoX(), clip.getLoY(),
			   clip.getHiX(), clip.getHiY()))
	{
	    // Problem using D3D; disable D3D in the rendering
	    // and force revalidation.
	    ((Win32OffScreenSurfaceData)sg2d.surfaceData).disableD3D();
	    sg2d.validatePipe();
	    super.drawLine(sg2d, x1, y1, x2, y2);
	}		
    }


    /**
     * draw a rectangle outline starting at x, y and going to the pixel
     * at (x + width), (y + width) (including the lower right pixel)
     */
    public void drawRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {	
	// REMIND: This check should probably go in SunGraphics2D instead.
	if (width < 0 || height < 0) {
	    return;
	}
	if (width == 0 || height == 0) {
	    // Handle degenerate cases through line renderer
	    drawLine(sg2d, x, y, (x + width), (y + height));
	    return;
	}
	int transx = x + sg2d.transX;
	int transy = y + sg2d.transY;
	Region clip = sg2d.getCompClip();
	if (!canHandleClipping && 
	    rectClipNeeded(clip, transx, transy, width, height)) 
	{
	    // Punt to superclass for clipped rects
	    super.drawRect(sg2d, x, y, width, height);
	    return;
	}
	if (!doDrawRectD3D(sg2d.surfaceData, sg2d.rgb, 
			   transx, transy,
			   width, height,
			   clip.getLoX(), clip.getLoY(),
			   clip.getHiX(), clip.getHiY()))
	{
	    // Problem using D3D; disable D3D in the rendering
	    // and force revalidation.
	    ((Win32OffScreenSurfaceData)sg2d.surfaceData).disableD3D();
	    sg2d.validatePipe();
	    super.drawRect(sg2d, x, y, width, height);
	}		
    }

    public Win32D3DRenderer traceWrapD3D() {
	return new Tracer(canHandleClipping);
    }

    public static class Tracer extends Win32D3DRenderer {
	Tracer(boolean canHandleClipping) {
	    super(canHandleClipping);
	}
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
	    GraphicsPrimitive.tracePrimitive("DDFillRect");
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
	boolean doDrawLineD3D(SurfaceData sData,
			      int color,
			      int x1, int y1, int x2, int y2,
			      int clipX1, int clipY1, int clipX2, int clipY2)
	{
	    // REMIND: Tracing does not work when the functions
	    // punt to superclass methods
	    GraphicsPrimitive.tracePrimitive("D3DDrawLine");
	    return super.doDrawLineD3D(sData, color, x1, y1, x2, y2,
	    			       clipX1, clipY1, clipX2, clipY2);
	}
	void doFillRectDD(SurfaceData sData,
			  int color,
			  int left, int top,
			  int right, int bottom)
	{
	    GraphicsPrimitive.tracePrimitive("DDFillRect");
	    super.doFillRectDD(sData, color, left, top, right, bottom);
	}
	boolean doDrawRectD3D(SurfaceData sData,
			  int color,
			  int x, int y, int w, int h,
			  int clipX1, int clipY1, int clipX2, int clipY2)
	{
	    // REMIND: Tracing does not work when the functions
	    // punt to superclass methods
	    GraphicsPrimitive.tracePrimitive("D3DDrawRect");
	    return super.doDrawRectD3D(sData, color, x, y, w, h,
	    			       clipX1, clipY1, clipX2, clipY2);
	}
    }
}

