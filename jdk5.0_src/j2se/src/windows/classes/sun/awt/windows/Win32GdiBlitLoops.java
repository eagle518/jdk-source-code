/*
 * @(#)Win32GdiBlitLoops.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Composite;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.Blit;
import sun.java2d.pipe.Region;
import sun.java2d.SurfaceData;

/**
 * Win32GdiBlitLoops
 * 
 * This class accelerates Blits between certain surfaces and the 
 * screen, using GDI.  The reason for these loops is to find
 * a way of copying to the screen without using DDraw locking
 * that is faster than our current fallback (which creates
 * a temporary GDI DIB)
 */
public class Win32GdiBlitLoops extends Blit {

    // Store these values to be passed to native code
    int rmask, gmask, bmask;

    // Needs lookup table (for indexed color image copies)
    boolean indexed = false;

    /**
     * Note that we do not register loops to 8-byte destinations.  This
     * is due to faster processing of dithering through our software 
     * loops than through GDI StretchBlt processing.
     */
    public static void register() 
    {	    
        GraphicsPrimitive[] primitives = {
	    new Win32GdiBlitLoops(SurfaceType.IntRgb,
				  Win32SurfaceData.AnyGdi),
	    new Win32GdiBlitLoops(SurfaceType.Ushort555Rgb,
				  Win32SurfaceData.AnyGdi,
				  0x7C00, 0x03E0, 0x001F),
	    new Win32GdiBlitLoops(SurfaceType.Ushort565Rgb,
				  Win32SurfaceData.AnyGdi,
				  0xF800, 0x07E0, 0x001F),
	    new Win32GdiBlitLoops(SurfaceType.ThreeByteBgr,
				  Win32SurfaceData.AnyGdi),
	    new Win32GdiBlitLoops(SurfaceType.ByteIndexedOpaque,
				  Win32SurfaceData.AnyGdi,
				  true),
	    new Win32GdiBlitLoops(SurfaceType.Index8Gray,
				  Win32SurfaceData.AnyGdi,
				  true),
	    new Win32GdiBlitLoops(SurfaceType.ByteGray,
				  Win32SurfaceData.AnyGdi),
	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    /**
     * This constructor exists for srcTypes that have no need of
     * component masks. GDI only expects masks for 2- and 4-byte 
     * DIBs, so all 1- and 3-byte srcTypes can skip the mask setting.
     */
    public Win32GdiBlitLoops(SurfaceType srcType, SurfaceType dstType) {
	this(srcType, dstType, 0, 0, 0);
    }

    /**
     * This constructor exists for srcTypes that need lookup tables
     * during image copying.
     */
    public Win32GdiBlitLoops(SurfaceType srcType, SurfaceType dstType, 
			     boolean indexed) {
	this(srcType, dstType, 0, 0, 0);
	this.indexed = indexed;
    }

    /**
     * This constructor sets mask for this primitive which can be
     * retrieved in native code to set the appropriate values for GDI.
     */
    public Win32GdiBlitLoops(SurfaceType srcType, SurfaceType dstType,
			     int rmask, int gmask, int bmask) {
	super(srcType, CompositeType.SrcNoEa, dstType);
	this.rmask = rmask;
	this.gmask = gmask;
	this.bmask = bmask;
    }

    /**
     * nativeBlit
     * This native method is where all of the work happens in the
     * accelerated Blit.
     */
    public native void nativeBlit(SurfaceData src, SurfaceData dst, 
				  Region clip,
				  int sx, int sy, int dx, int dy, 
				  int w, int h, 
				  int rmask, int gmask, int bmask,
				  boolean needLut);

    /**
     * Blit
     * This method wraps the nativeBlit call, sending in additional
     * info on whether the native method needs to get LUT info
     * from the source image.  Note that we do not pass in the
     * Composite data because we only register these loops for
     * SrcNoEa composite operations.
     */
    public void Blit(SurfaceData src, SurfaceData dst, 
		     Composite comp, Region clip,
		     int sx, int sy, int dx, int dy, int w, int h) 
    {
	nativeBlit(src, dst, clip, sx, sy, dx, dy, w, h, 
		   rmask, gmask, bmask, indexed);
    }


}
