/*
 * @(#)GLXRemoteOffScreenImage.java	1.6 04/02/17
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
import sun.awt.X11ComponentPeer;
import sun.awt.X11SurfaceData;
import sun.awt.image.BufImgSurfaceData;
import sun.awt.image.RemoteOffScreenImage;
import sun.awt.image.SurfaceManager;
import sun.awt.motif.X11CachingSurfaceManager;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.Blit;

/**
 * This is the GLX-specific variant of RemoteOffScreenImage, useful for
 * providing a hardware accelerated backbuffer via Component.createImage().
 */
public class GLXRemoteOffScreenImage extends RemoteOffScreenImage {

    /**
     * Intermediate BufImgSD for use in copyDefaultToAccelerated.
     */
    private SurfaceData bisd;

    public GLXRemoteOffScreenImage(Component c,
                                   ColorModel cm, WritableRaster raster,
				   boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
        createNativeRaster();
    }

    protected SurfaceManager createSurfaceManager() {
        return new GLXRemoteSurfaceManager(this);
    }

    private class GLXRemoteSurfaceManager extends X11CachingSurfaceManager {

        public GLXRemoteSurfaceManager(BufferedImage bi) {
            super(bi);

            if (!accelerationEnabled) {
                return;
            }

            GraphicsConfiguration gc = 
                GLXSurfaceData.getGC(c == null ?
                                     null :
                                     (X11ComponentPeer)c.getPeer());
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
        public SurfaceData getSourceSurfaceData(SurfaceData destSD,
                                                CompositeType comp,
                                                Color bgColor,
                                                boolean scale)
        {
            if (accelerationEnabled       &&
                (destSD != sdAccel)       &&
                isDestSurfaceAccelerated(destSD))
            {
                // First, we validate the pixmap sd if necessary and then
                // return the appropriate surfaceData object.
                validate(destSD.getDeviceConfiguration());
                if (sdAccel != null) {
                    return sdAccel;
                }
            }
            return sdDefault;
        }

        protected SurfaceData createGLXSurface(GraphicsConfiguration gc,
                                               int width, int height)
        {
            return GLXSurfaceData.createData((GLXGraphicsConfig)gc,
                                             width, height,
                                             gc.getColorModel(), bImg,
                                             OGLSurfaceData.PBUFFER);
        }

        protected void copyDefaultToAccelerated() {
            if (sdDefault != null && sdAccel != null &&
                sdDefault != sdAccel)
            {
                // REMIND: we are able to copy from hw surface to hw
                //         surface (unlike the X11 pipeline, which has this
                //         restriction)... this needs to be fixed...
                if (bisd == null) {
		    BufferedImage bi;
		    if (bufImageTypeSw > BufferedImage.TYPE_CUSTOM) {
			bi = new BufferedImage(getWidth(), getHeight(), 
					       bufImageTypeSw);
	    
		    } else {
			ColorModel cm = getColorModel();
			WritableRaster wr = 
			    cm.createCompatibleWritableRaster(getWidth(), getHeight());
			bi = new BufferedImage(cm, wr,
					   cm.isAlphaPremultiplied(), null);
		    }
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
