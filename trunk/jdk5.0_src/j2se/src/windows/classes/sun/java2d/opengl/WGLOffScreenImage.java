/*
 * @(#)WGLOffScreenImage.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Component;
import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.AlphaComposite;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;
import sun.awt.image.BufImgSurfaceData;
import sun.awt.image.RemoteOffScreenImage;
import sun.awt.image.SurfaceManager;
import sun.awt.windows.WComponentPeer;
import sun.java2d.SurfaceData;
import sun.java2d.loops.Blit;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;

/**
 * This is the WGL-specific variant of RemoteOffScreenImage, useful for
 * providing a hardware accelerated backbuffer via Component.createImage().
 * Note that this class uses the same concepts that are used for
 * GLXRemoteOffScreenImage.  This type of image is important for applications
 * that expect an image returned from Component.createImage() to provide
 * optimal performance for doublebuffering situations.
 */
public class WGLOffScreenImage extends RemoteOffScreenImage {

    /**
     * Intermediate BufImgSD for use in copyDefaultToAccelerated.
     */
    private SurfaceData bisd;

    public WGLOffScreenImage(Component c,
                             ColorModel cm, WritableRaster raster,
                             boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
        createNativeRaster();
    }

    protected SurfaceManager createSurfaceManager() {
        return new WGLOffScreenSurfaceManager(this);
    }

    private class WGLOffScreenSurfaceManager extends WGLCachingSurfaceManager {

        public WGLOffScreenSurfaceManager(BufferedImage bi) {
            super(bi);

            if (!accelerationEnabled) {
                return;
            }

            GraphicsConfiguration gc = 
                WGLSurfaceData.getGC(c == null ?
                                     null :
                                     (WComponentPeer)c.getPeer());
            initAcceleratedSurface(gc, getWidth(), getHeight());
            if (sdAccel != null) {
                // This is the trick: we treat sdAccel as the default
                // software SurfaceData
                sdDefault = sdAccel;
            }
        }

        /**
         * Need to override this method as we don't need to check for
         * the number of copies done from this image
         */
        @Override
        public SurfaceData getSourceSurfaceData(SurfaceData destSD,
                                                CompositeType comp,
                                                Color bgColor,
                                                boolean scale)
        {
            if (accelerationEnabled       &&
                (destSD != sdAccel)       &&
                isDestSurfaceAccelerated(destSD))
            {
                // First, we validate the pbuffer surface if necessary and
                // then return the appropriate SurfaceData object.
                validate(destSD.getDeviceConfiguration());
                if (sdAccel != null) {
                    return sdAccel;
                }
            }
            return sdDefault;
        }

        @Override
        protected SurfaceData createAccelSurface(GraphicsConfiguration gc,
                                                 int width, int height)
        {
            return WGLSurfaceData.createData((WGLGraphicsConfig)gc,
                                             width, height,
                                             gc.getColorModel(), bImg,
                                             OGLSurfaceData.PBUFFER);
        }

        @Override
        protected void copyDefaultToAccelerated() {
            if (sdDefault != null && sdAccel != null &&
                sdDefault != sdAccel)
            {
                if (bisd == null) {
                    BufferedImage bi = new BufferedImage(getWidth(),
                                                         getHeight(),
                                                         bufImageTypeSw);
                    bisd = BufImgSurfaceData.createData(bi);
                }

                SurfaceType srcType = sdDefault.getSurfaceType();
                SurfaceType biType = bisd.getSurfaceType();
                SurfaceType dstType = sdAccel.getSurfaceType();

		Blit blit = Blit.getFromCache(srcType,
					      CompositeType.SrcNoEa,
					      biType);
		blit.Blit(sdDefault, bisd,
			  AlphaComposite.Src, null,
			  0, 0, 0, 0,
			  getWidth(), getHeight());

		blit = Blit.getFromCache(biType,
					 CompositeType.SrcNoEa,
					 dstType);
		blit.Blit(bisd, sdAccel,
			  AlphaComposite.Src, null,
			  0, 0, 0, 0,
			  getWidth(), getHeight());
            }
        }
    }
}
