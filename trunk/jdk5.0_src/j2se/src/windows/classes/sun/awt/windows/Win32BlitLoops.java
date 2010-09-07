/*
 * @(#)Win32BlitLoops.java	1.18 03/12/19
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
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.pipe.Region;
import sun.awt.image.BufImgSurfaceData;

/**
 * Win32BlitLoops
 * 
 * This class accelerates Blits between two DD surfaces of the same depth, 
 * using DirectDraw commands in native code to dispatch Blt calls to 
 * the video accelerator.
 */
public class Win32BlitLoops extends Blit {

    public static void register() 
    {
        GraphicsPrimitive[] primitives = {
	    // opaque to opaque loops

	    // Note that we use the offscreen surfaceType for
	    // this registry, but that both the onscreen and offscreen
	    // DD types use the same DESC strings, so these loops will
	    // be used whether the src/dest surfaces are onscreen or not
	    new Win32BlitLoops(Win32SurfaceData.IntRgbDD,
			       Win32SurfaceData.IntRgbDD, false),
	    new Win32BlitLoops(Win32SurfaceData.Ushort565RgbDD,
			       Win32SurfaceData.Ushort565RgbDD, false),
	    new Win32BlitLoops(Win32SurfaceData.IntRgbxDD,
			       Win32SurfaceData.IntRgbxDD, false),
	    new Win32BlitLoops(Win32SurfaceData.Ushort555RgbxDD,
			       Win32SurfaceData.Ushort555RgbxDD, false),
	    new Win32BlitLoops(Win32SurfaceData.Ushort555RgbDD,
			       Win32SurfaceData.Ushort555RgbDD, false),
	    new Win32BlitLoops(Win32SurfaceData.ByteIndexedOpaqueDD,
			       Win32SurfaceData.ByteIndexedOpaqueDD, false),
	    new Win32BlitLoops(Win32SurfaceData.ByteGrayDD,
			       Win32SurfaceData.ByteGrayDD, false),
	    new Win32BlitLoops(Win32SurfaceData.Index8GrayDD,
			       Win32SurfaceData.Index8GrayDD, false),
	    new Win32BlitLoops(Win32SurfaceData.ThreeByteBgrDD, 
			       Win32SurfaceData.ThreeByteBgrDD, false),

	    // 1-bit transparent to opaque loops
	    new Win32BlitLoops(Win32SurfaceData.IntRgbDD_BM,
			       Win32SurfaceData.IntRgbDD, true),
	    new Win32BlitLoops(Win32SurfaceData.Ushort565RgbDD_BM,
			       Win32SurfaceData.Ushort565RgbDD, true),
	    new Win32BlitLoops(Win32SurfaceData.IntRgbxDD_BM,
			       Win32SurfaceData.IntRgbxDD, true),
	    new Win32BlitLoops(Win32SurfaceData.Ushort555RgbxDD_BM,
			       Win32SurfaceData.Ushort555RgbxDD, true),
	    new Win32BlitLoops(Win32SurfaceData.Ushort555RgbDD_BM,
			       Win32SurfaceData.Ushort555RgbDD, true),
	    new Win32BlitLoops(Win32SurfaceData.ByteIndexedDD_BM,
			       Win32SurfaceData.ByteIndexedOpaqueDD, true),
	    new Win32BlitLoops(Win32SurfaceData.ByteGrayDD_BM,
			       Win32SurfaceData.ByteGrayDD, true),
	    new Win32BlitLoops(Win32SurfaceData.Index8GrayDD_BM,
			       Win32SurfaceData.Index8GrayDD, true),
	    new Win32BlitLoops(Win32SurfaceData.ThreeByteBgrDD_BM, 
			       Win32SurfaceData.ThreeByteBgrDD, true),

	    // any to 1-bit transparent bg loops
	    new DelegateBlitBgLoop(Win32SurfaceData.IntRgbDD_BM,
				   Win32SurfaceData.IntRgbDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.Ushort565RgbDD_BM,
				   Win32SurfaceData.Ushort565RgbDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.IntRgbxDD_BM,
				   Win32SurfaceData.IntRgbxDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.Ushort555RgbxDD_BM,
				   Win32SurfaceData.Ushort555RgbxDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.Ushort555RgbDD_BM,
				   Win32SurfaceData.Ushort555RgbDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.ByteIndexedDD_BM,
				   Win32SurfaceData.ByteIndexedOpaqueDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.ByteGrayDD_BM,
				   Win32SurfaceData.ByteGrayDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.Index8GrayDD_BM,
				   Win32SurfaceData.Index8GrayDD),
	    new DelegateBlitBgLoop(Win32SurfaceData.ThreeByteBgrDD_BM, 
				   Win32SurfaceData.ThreeByteBgrDD),

	};
	GraphicsPrimitiveMgr.register(primitives);
    }

    public Win32BlitLoops(SurfaceType srcType, SurfaceType dstType, boolean over) {
	super(srcType, 
	      over ? CompositeType.SrcOverNoEa : CompositeType.SrcNoEa, 
	      dstType);
    }

    /**
     * Blit
     * This native method is where all of the work happens in the
     * accelerated Blit.
     */
    public native void Blit(SurfaceData src, SurfaceData dst, 
			    Composite comp, Region clip,
			    int sx, int sy, int dx, int dy, int w, int h);


    /**
     * BlitBg
     * This loop is used to render from Sw surface data
     * to the Hw one in AOSI.copyBackupToAccelerated.
     */
    static class DelegateBlitBgLoop extends BlitBg {
	SurfaceType dstType;
	private static final Font defaultFont = new Font("Dialog", Font.PLAIN, 12);

	public DelegateBlitBgLoop(SurfaceType realDstType, SurfaceType delegateDstType) {
	    super(SurfaceType.Any, CompositeType.SrcNoEa, realDstType);
	    this.dstType = delegateDstType;
	}

	public void BlitBg(SurfaceData srcData, SurfaceData dstData, 
			   Composite comp, Region clip, Color bgColor,
			   int srcx, int srcy, int dstx, int dsty, int width, int height) 
	{
	    ColorModel dstModel = dstData.getColorModel();
	    WritableRaster wr =
		dstModel.createCompatibleWritableRaster(width, height);
	    boolean isPremult = dstModel.isAlphaPremultiplied();
	    BufferedImage bimg =
		new BufferedImage(dstModel, wr, isPremult, null);
	    SurfaceData tmpData = BufImgSurfaceData.createData(bimg);
	    SunGraphics2D sg2d = new SunGraphics2D(tmpData, bgColor, bgColor,
						   defaultFont);
	    FillRect fillop = FillRect.locate(SurfaceType.AnyColor,
					      CompositeType.SrcNoEa,
					      tmpData.getSurfaceType());
	    Blit combineop = Blit.getFromCache(srcData.getSurfaceType(),
					       CompositeType.SrcOverNoEa,
					       tmpData.getSurfaceType());
	    Blit blitop = Blit.getFromCache(tmpData.getSurfaceType(),
					    CompositeType.SrcNoEa, dstType);
	    fillop.FillRect(sg2d, tmpData, 0, 0, width, height);
	    combineop.Blit(srcData, tmpData, AlphaComposite.SrcOver, null,
			   srcx, srcy, 0, 0, width, height);
	    blitop.Blit(tmpData, dstData, comp, clip,
			0, 0, dstx, dsty, width, height);
	}
    }
}
