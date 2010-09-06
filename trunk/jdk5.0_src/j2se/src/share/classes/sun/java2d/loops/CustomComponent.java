/*
 * @(#)CustomComponent.java	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 * @author Jim Graham
 */

package sun.java2d.loops;

import java.awt.Composite;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import sun.awt.image.IntegerComponentRaster;
import sun.java2d.SurfaceData;
import sun.java2d.pipe.Region;
import sun.java2d.pipe.SpanIterator;

/**
 *   CustomComponent, collection of GraphicsPrimitive
 *   Basically, this collection of components performs conversion from
 *   ANY to ANY via opaque copy
 */
public final class CustomComponent {
    public static void register() {
	// REMIND: This does not work for all destinations yet since
	// the screen SurfaceData objects do not implement getRaster
	Class owner = CustomComponent.class;
        GraphicsPrimitive[] primitives = {
	    new GraphicsPrimitiveProxy(owner, "OpaqueCopyAnyToArgb",
				       Blit.methodSignature,
				       Blit.primTypeID,
				       SurfaceType.Any,
				       CompositeType.SrcNoEa,
				       SurfaceType.IntArgb),
	    new GraphicsPrimitiveProxy(owner, "OpaqueCopyArgbToAny",
				       Blit.methodSignature,
				       Blit.primTypeID,
				       SurfaceType.IntArgb,
				       CompositeType.SrcNoEa,
				       SurfaceType.Any),
	    new GraphicsPrimitiveProxy(owner, "XorCopyArgbToAny",
				       Blit.methodSignature,
				       Blit.primTypeID,
				       SurfaceType.IntArgb,
				       CompositeType.Xor,
				       SurfaceType.Any),
	};
	GraphicsPrimitiveMgr.register(primitives);
    }
}

/**
 *   ANY format to ARGB format Blit
 */
class OpaqueCopyAnyToArgb extends Blit {
    OpaqueCopyAnyToArgb() {
	super(SurfaceType.Any,
	      CompositeType.SrcNoEa,
	      SurfaceType.IntArgb);
    }

    public void Blit(SurfaceData src, SurfaceData dst,
		     Composite comp, Region clip,
		     int srcx, int srcy, int dstx, int dsty, int w, int h)
    {
	Raster srcRast = src.getRaster(srcx, srcy, w, h);
	ColorModel srcCM = src.getColorModel();

	Raster dstRast = dst.getRaster(dstx, dsty, w, h);
	IntegerComponentRaster icr = (IntegerComponentRaster) dstRast;
	int[] dstPix = icr.getDataStorage();

	/*
	 * REMIND: intersect bboxes...
	 * w = Math.min(srcRast.getWidth()-srcXOffs, w);
	 * w = Math.min(dstRast.getWidth()-dstXOffs, w);
	 * h = Math.min(srcRast.getHeight()-srcYOffs, h);
	 * h = Math.min(dstRast.getHeight()-dstYOffs, h);
	 */
	if (clip == null) {
	    clip = Region.getInstanceXYWH(dstx, dsty, w, h);
	}
	int span[] = {dstx, dsty, dstx+w, dsty+h};
	SpanIterator si = clip.getSpanIterator(span);

	Object srcPix = null;

	int dstScan = icr.getScanlineStride();
	// assert(icr.getPixelStride() == 1);
	srcx -= dstx;
	srcy -= dsty;
	while (si.nextSpan(span)) {
	    int rowoff = icr.getDataOffset(0) + span[1] * dstScan + span[0];
	    for (int y = span[1]; y < span[3]; y++) {
		int off = rowoff;
		for (int x = span[0]; x < span[2]; x++) {
		    srcPix = srcRast.getDataElements(x+srcx, y+srcy, srcPix);
		    dstPix[off++] = srcCM.getRGB(srcPix);
		}
		rowoff += dstScan;
	    }
	}
	// REMIND: We need to do something to make sure that dstRast
	// is put back to the destination (as in the native Release
	// function)
	// src.releaseRaster(srcRast);	// NOP?
	// dst.releaseRaster(dstRast);
    }
}

/**
 *   ARGB format to ANY format Blit
 */
class OpaqueCopyArgbToAny extends Blit {
    OpaqueCopyArgbToAny() {
	super(SurfaceType.IntArgb,
	      CompositeType.SrcNoEa,
	      SurfaceType.Any);
    }

    public void Blit(SurfaceData src, SurfaceData dst,
		     Composite comp, Region clip,
		     int srcx, int srcy, int dstx, int dsty, int w, int h)
    {
	Raster srcRast = src.getRaster(srcx, srcy, w, h);
	IntegerComponentRaster icr = (IntegerComponentRaster) srcRast;
	int[] srcPix = icr.getDataStorage();

	WritableRaster dstRast =
	    (WritableRaster) dst.getRaster(dstx, dsty, w, h);
	ColorModel dstCM = dst.getColorModel();

	/*
	 * REMIND: intersect bboxes...
	 * w = Math.min(srcRast.getWidth()-srcXOffs, w);
	 * w = Math.min(dstRast.getWidth()-dstXOffs, w);
	 * h = Math.min(srcRast.getHeight()-srcYOffs, h);
	 * h = Math.min(dstRast.getHeight()-dstYOffs, h);
	 */
	if (clip == null) {
	    clip = Region.getInstanceXYWH(dstx, dsty, w, h);
	}
	int span[] = {dstx, dsty, dstx+w, dsty+h};
	SpanIterator si = clip.getSpanIterator(span);

	Object dstPix = null;

	int srcScan = icr.getScanlineStride();
	// assert(icr.getPixelStride() == 1);
	srcx -= dstx;
	srcy -= dsty;
	while (si.nextSpan(span)) {
	    int rowoff = (icr.getDataOffset(0) +
			  (srcy + span[1]) * srcScan +
			  (srcx + span[0]));
	    for (int y = span[1]; y < span[3]; y++) {
		int off = rowoff;
		for (int x = span[0]; x < span[2]; x++) {
		    dstPix = dstCM.getDataElements(srcPix[off++], dstPix);
		    dstRast.setDataElements(x, y, dstPix);
		}
		rowoff += srcScan;
	    }
	}
	// REMIND: We need to do something to make sure that dstRast
	// is put back to the destination (as in the native Release
	// function)
	// src.releaseRaster(srcRast);	// NOP?
	// dst.releaseRaster(dstRast);
    }
}

/**
 *   ARGB format to ANY format Blit (pixels are XORed together with XOR pixel)
 */
class XorCopyArgbToAny extends Blit {
    XorCopyArgbToAny() {
	super(SurfaceType.IntArgb,
	      CompositeType.Xor,
	      SurfaceType.Any);
    }

    public void Blit(SurfaceData src, SurfaceData dst,
		     Composite comp, Region clip,
		     int srcx, int srcy, int dstx, int dsty, int w, int h)
    {
	Raster srcRast = src.getRaster(srcx, srcy, w, h);
	IntegerComponentRaster icr = (IntegerComponentRaster) srcRast;
	int[] srcPix = icr.getDataStorage();

	WritableRaster dstRast =
	    (WritableRaster) dst.getRaster(dstx, dsty, w, h);
	ColorModel dstCM = dst.getColorModel();

	/*
	 * REMIND: intersect bboxes...
	 * w = Math.min(srcRast.getWidth()-srcXOffs, w);
	 * w = Math.min(dstRast.getWidth()-dstXOffs, w);
	 * h = Math.min(srcRast.getHeight()-srcYOffs, h);
	 * h = Math.min(dstRast.getHeight()-dstYOffs, h);
	 */
	if (clip == null) {
	    clip = Region.getInstanceXYWH(dstx, dsty, w, h);
	}
	int span[] = {dstx, dsty, dstx+w, dsty+h};
	SpanIterator si = clip.getSpanIterator(span);

        int xorrgb = ((XORComposite)comp).getXorColor().getRGB();
        Object xorPixel = dstCM.getDataElements(xorrgb, null);

	Object srcPixel = null;
	Object dstPixel = null;

	int srcScan = icr.getScanlineStride();
	// assert(icr.getPixelStride() == 1);
	srcx -= dstx;
	srcy -= dsty;
	while (si.nextSpan(span)) {
	    int rowoff = (icr.getDataOffset(0) +
			  (srcy + span[1]) * srcScan +
			  (srcx + span[0]));
	    for (int y = span[1]; y < span[3]; y++) {
		int off = rowoff;
		for (int x = span[0]; x < span[2]; x++) {
		    // REMIND: alpha bits of the destination pixel are
		    // currently altered by the XOR operation, but
		    // should be left untouched
		    srcPixel = dstCM.getDataElements(srcPix[off++], srcPixel);
		    dstPixel = dstRast.getDataElements(x, y, dstPixel);

		    switch (dstCM.getTransferType()) {
		    case DataBuffer.TYPE_BYTE:
			byte[] bytesrcarr = (byte[]) srcPixel;
			byte[] bytedstarr = (byte[]) dstPixel;
			byte[] bytexorarr = (byte[]) xorPixel;
			for (int i = 0; i < bytedstarr.length; i++) {
			    bytedstarr[i] ^= bytesrcarr[i] ^ bytexorarr[i];
			}
			break; 
		    case DataBuffer.TYPE_SHORT:
		    case DataBuffer.TYPE_USHORT:
			short[] shortsrcarr = (short[]) srcPixel;
			short[] shortdstarr = (short[]) dstPixel;
			short[] shortxorarr = (short[]) xorPixel;
			for (int i = 0; i < shortdstarr.length; i++) {
			    shortdstarr[i] ^= shortsrcarr[i] ^ shortxorarr[i];
			}
			break;
		    case DataBuffer.TYPE_INT:
			int[] intsrcarr = (int[]) srcPixel;
			int[] intdstarr = (int[]) dstPixel;
			int[] intxorarr = (int[]) xorPixel;
			for (int i = 0; i < intdstarr.length; i++) {
			    intdstarr[i] ^= intsrcarr[i] ^ intxorarr[i];
			}
			break;
		    case DataBuffer.TYPE_FLOAT:
			float[] floatsrcarr = (float[]) srcPixel;
			float[] floatdstarr = (float[]) dstPixel;
			float[] floatxorarr = (float[]) xorPixel;
			for (int i = 0; i < floatdstarr.length; i++) {
			    int v = (Float.floatToIntBits(floatdstarr[i]) ^
				     Float.floatToIntBits(floatsrcarr[i]) ^
				     Float.floatToIntBits(floatxorarr[i]));
			    floatdstarr[i] = Float.intBitsToFloat(v);
			}
			break;
		    case DataBuffer.TYPE_DOUBLE:
			double[] doublesrcarr = (double[]) srcPixel;
			double[] doubledstarr = (double[]) dstPixel;
			double[] doublexorarr = (double[]) xorPixel;
			for (int i = 0; i < doubledstarr.length; i++) {
			    long v = (Double.doubleToLongBits(doubledstarr[i]) ^
				      Double.doubleToLongBits(doublesrcarr[i]) ^
				      Double.doubleToLongBits(doublexorarr[i]));
			    doubledstarr[i] = Double.longBitsToDouble(v);
			}
			break;
		    default:
			throw new InternalError("Unsupported XOR pixel type");
		    }
		    dstRast.setDataElements(x, y, dstPixel);
		}
		rowoff += srcScan;
	    }
	}
	// REMIND: We need to do something to make sure that dstRast
	// is put back to the destination (as in the native Release
	// function)
	// src.releaseRaster(srcRast);	// NOP?
	// dst.releaseRaster(dstRast);
    }
}
