/*
 * @(#)WCustomCursor.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import sun.awt.CustomCursor;
import java.awt.*;
import java.awt.image.*;
import sun.awt.image.ImageRepresentation;
import sun.awt.image.IntegerComponentRaster;
import sun.awt.image.ToolkitImage;

/**
 * A class to encapsulate a custom image-based cursor.  
 *
 * @see Component#setCursor
 * @version 	1.15, 12/19/03
 * @author 	ThomasBall
 */
public class WCustomCursor extends CustomCursor {

    public WCustomCursor(Image cursor, Point hotSpot, String name) 
            throws IndexOutOfBoundsException {
        super(cursor, hotSpot, name);
    }

    protected void createNativeCursor(Image im, int[] pixels, int w, int h,
                                      int xHotSpot, int yHotSpot) {
	BufferedImage bimage = new BufferedImage(w, h,
                               BufferedImage.TYPE_INT_RGB);
        Graphics g = bimage.getGraphics();
	try {
	    if (im instanceof ToolkitImage) {
	        ImageRepresentation ir = ((ToolkitImage)im).getImageRep();
		ir.reconstruct(ImageObserver.ALLBITS);
	    }
	    g.drawImage(im, 0, 0, w, h, null);
	} finally {
	    g.dispose();
	}
        Raster  raster = bimage.getRaster();
        DataBuffer buffer = raster.getDataBuffer();
        // REMIND: native code should use ScanStride _AND_ width
        int data[] = ((DataBufferInt)buffer).getData();

        byte[] andMask = new byte[w * h / 8];
        int npixels = pixels.length;
        for (int i = 0; i < npixels; i++) {
            int ibyte = i / 8;
            int omask = 1 << (7 - (i % 8));
            if ((pixels[i] & 0xff000000) == 0) {
                // Transparent bit
                andMask[ibyte] |= omask;
	    }
	}

        {
            int     ficW = raster.getWidth();
            if( raster instanceof IntegerComponentRaster ) {
                ficW = ((IntegerComponentRaster)raster).getScanlineStride();
            }
            createCursorIndirect(
                ((DataBufferInt)bimage.getRaster().getDataBuffer()).getData(),
		andMask, ficW, raster.getWidth(), raster.getHeight(),
                xHotSpot, yHotSpot);
        }
    }

    private native void createCursorIndirect(int[] rData, byte[] andMask,
					     int nScanStride, int width,
					     int height, int xHotSpot,
					     int yHotSpot);
    /**
     * Return the current value of SM_CXCURSOR.
     */
    static native int getCursorWidth();

    /**
     * Return the current value of SM_CYCURSOR.
     */
    static native int getCursorHeight();
}
