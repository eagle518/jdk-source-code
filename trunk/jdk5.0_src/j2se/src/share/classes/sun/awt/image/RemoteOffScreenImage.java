/*
 * @(#)RemoteOffScreenImage.java	1.4 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Component;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DirectColorModel;
import java.awt.image.PixelInterleavedSampleModel;
import java.awt.image.SampleModel;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.WritableRaster;
import sun.awt.image.BufImgSurfaceData;
import sun.awt.image.DataBufferNative;
import sun.awt.image.WritableRasterNative;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;
import sun.java2d.pipe.DrawImage;

/**
 * This class extends the functionality of OffScreenImage by forcing the
 * use of an accelerated surface by default.  It is intended to be used as
 * the base class for other platform-specific variants (X11, GLX) that
 * specify their own customized CachingSurfaceManager for handling the
 * details of surface management.
 */
public class RemoteOffScreenImage extends OffScreenImage {

    /**
     * Cache for later BI creation in copyDefaultToAccelerated.
     */
    protected int bufImageTypeSw;

    private static native void initIDs();

    static {
	initIDs();
    }

    public RemoteOffScreenImage(Component c,
                                ColorModel cm, WritableRaster raster,
                                boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
	this.bufImageTypeSw = getType();
    }

    private native void	setRasterNative(WritableRaster raster);

    protected void createNativeRaster() {
	WritableRasterNative wrn = 
	    WritableRasterNative.createNativeRaster(getColorModel(), 
						    surfaceManager.getDestSurfaceData(),
						    getWidth(), getHeight());
	setRasterNative(wrn);
    }

    /**
     * Returns a plain BufferedImage representation of this image.  Useful
     * in situations (such as the medialib code) which require an image
     * containing a "real" WritableRaster and actual pixel data.
     */
    public BufferedImage getSnapshot() {
	BufferedImage bImg;
	if (bufImageTypeSw > BufferedImage.TYPE_CUSTOM) {
	    bImg = new BufferedImage(getWidth(), getHeight(), 
				bufImageTypeSw);
	    
	} else {
	    ColorModel cm = getColorModel();
	    WritableRaster wr = 
		cm.createCompatibleWritableRaster(getWidth(), getHeight());
            bImg = new BufferedImage(cm, wr,
                                     cm.isAlphaPremultiplied(), null);
	}
	Graphics2D g = bImg.createGraphics();
	g.drawImage(this, 0, 0, null);
	g.dispose();
	return bImg;
    }
}
