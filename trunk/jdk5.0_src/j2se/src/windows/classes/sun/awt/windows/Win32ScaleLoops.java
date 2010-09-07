/*
 * @(#)Win32ScaleLoops.java	1.8 04/01/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.GraphicsPrimitiveMgr;
import sun.java2d.loops.GraphicsPrimitiveProxy;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.ScaledBlit;
import sun.java2d.pipe.Region;
import sun.java2d.SurfaceData;
import java.awt.Composite;

/**
 * Win32ScaleLoops
 * 
 * This class accelerates ScaledBlits between two surfaces of type IntRgbDD.  Since
 * the onscreen surface is of that type and some of the offscreen surfaces
 * may be of that type (if they were created via FastOffScreenImage), then
 * this type of ScaledBlit will accelerated double-buffer copies between those
 * two surfaces.
*/
public class Win32ScaleLoops extends ScaledBlit {
    private ScaledBlit swblit;

    public static void register() 
    {
        GraphicsPrimitive[] primitives = {
	    new Win32ScaleLoops(Win32SurfaceData.IntRgbDD),
	    new Win32ScaleLoops(Win32SurfaceData.Ushort565RgbDD),
	    new Win32ScaleLoops(Win32SurfaceData.IntRgbxDD),
	    new Win32ScaleLoops(Win32SurfaceData.Ushort555RgbxDD),
	    new Win32ScaleLoops(Win32SurfaceData.Ushort555RgbDD),
	    new Win32ScaleLoops(Win32SurfaceData.ByteIndexedOpaqueDD),
	    new Win32ScaleLoops(Win32SurfaceData.ThreeByteBgrDD)
	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    public Win32ScaleLoops(SurfaceType surfType) {
	super(surfType, CompositeType.SrcNoEa, surfType);
    }

    /**
     * Scale
     * This native method is where all of the work happens in the
     * accelerated ScaledBlit for the scaling case.
     */
    public native void Scale(SurfaceData src, SurfaceData dst, 
			     Composite comp, int sx, int sy, 
			     int dx, int dy, int sw, int sh,
			     int dw, int dh);

    public void Scale(SurfaceData src, SurfaceData dst,
		      Composite comp, Region clip,
		      int sx, int sy,
		      int dx, int dy,
		      int sw, int sh,
		      int dw, int dh)
    {
	// REMIND: We can still do it if the clip equals the device
	// bounds for a destination window, but this logic rejects
	// that case...
	if (clip.encompassesXYWH(dx, dy, dw, dh)) {
	    Scale(src, dst, comp, sx, sy, dx, dy, sw, sh, dw, dh);
	} else {
	    if (swblit == null) {
		// REMIND: This assumes that the DD surface types are
		// directly derived from a non-DD type that has a loop.
		swblit = ScaledBlit.getFromCache(getSourceType().getSuperType(),
						 getCompositeType(),
						 getDestType().getSuperType());
	    }
	    swblit.Scale(src, dst, comp, clip,
			 sx, sy, dx, dy,
			 sw, sh, dw, dh);
	}
    }
}
