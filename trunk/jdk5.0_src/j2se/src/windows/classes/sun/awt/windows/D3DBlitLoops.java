/*
 * @(#)D3DBlitLoops.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Font;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;
import java.awt.image.BufferedImage;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.Blit;
import sun.java2d.loops.BlitBg;
import sun.java2d.loops.FillRect;
import sun.java2d.pipe.Region;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.awt.image.BufImgSurfaceData;

/**
 * D3DBlitLoops
 * 
 * This class accelerates Blits between a translucent D3D surface
 * and DD surface using Direct3D DrawPrimitive command.
 */
public class D3DBlitLoops extends Blit {

    public static void register() 
    {
        GraphicsPrimitive[] primitives = {
	    // loops for Int Argb
	    new D3DBlitLoops(Win32SurfaceData.IntArgbD3D,
			     Win32SurfaceData.IntRgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.IntArgbD3D,
			     Win32SurfaceData.Ushort565RgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.IntArgbD3D,
			     Win32SurfaceData.IntRgbxD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.IntArgbD3D,
			     Win32SurfaceData.Ushort555RgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.IntArgbD3D,
			     Win32SurfaceData.ThreeByteBgrD3D, CompositeType.SrcOver),

	    // loops for 4444 Argb
	    new D3DBlitLoops(Win32SurfaceData.Ushort4444ArgbD3D,
			     Win32SurfaceData.IntRgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.Ushort4444ArgbD3D,
			     Win32SurfaceData.Ushort565RgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.Ushort4444ArgbD3D,
			     Win32SurfaceData.IntRgbxD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.Ushort4444ArgbD3D,
			     Win32SurfaceData.Ushort555RgbD3D, CompositeType.SrcOver),
	    new D3DBlitLoops(Win32SurfaceData.Ushort4444ArgbD3D,
			     Win32SurfaceData.ThreeByteBgrD3D, CompositeType.SrcOver),
	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    public D3DBlitLoops(SurfaceType srcType, SurfaceType dstType, CompositeType comp) {
	super(srcType, comp, dstType);
    }

    /**
     * Blit
     * This native method is where all of the work happens in the
     * accelerated Blit.
     */
    public native void Blit(SurfaceData src, SurfaceData dst, 
			    Composite comp, Region clip,
			    int sx, int sy, int dx, int dy, int w, int h);

}
