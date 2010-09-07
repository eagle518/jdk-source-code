/*
 * @(#)X11RemoteOffScreenImage.java	1.21 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

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
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.CompositeType;
import sun.java2d.loops.Blit;

/**
 * This class extends the functionality of RemoteOffScreenImage by
 * forcing the use of a pixmap-based SurfaceData object.  In
 * X11CachingSurfaceManager, the default SurfaceData object is the default
 * BufImgSurfaceData, setup by a call to the superclass
 * constructor.  Acceleration is achieved by caching a version of
 * the data in a pixmap and copying from that pixmap when appropriate.
 * <P>
 * This is sufficient for accelerating sprite-type images, which are
 * rendered to infrequently but copied from often.  Back buffers 
 * do not benefit from this acceleration since the cached copy
 * would never be used.  This is deemed sufficient for local X
 * usage; users performing complex Java2D operations (e.g., 
 * anti-aliasing, compositing, text, or anything that requires
 * read-modify-write operations) would see a degradation in performance
 * were we to force all operations to go through the pixmap-based
 * image.
 * <P>
 * In the RemoteX case, however, performance is so abysmal in the
 * case of double-buffering (with the back buffer living in the 
 * Java heap), that the advantage of speeding up the basic 
 * applications (i.e., those using 1.1 API or simple Swing operations,
 * and thus avoiding complex Java2D read-modify-write operations)
 * is judged to outweigh the possible performance loss in some
 * applications from having the back buffer located in a pixmap.
 * <P>
 * The decision to instantiate this class (versus OffScreenImage)
 * is based on whether we are running remotely and whether the
 * user has enabled/disabled a property related to this issue:
 * -Dsun.java2d.pmoffscreen
 */
public class X11RemoteOffScreenImage extends RemoteOffScreenImage {

    /**
     * Intermediate BufImgSD for use in copyDefaultToAccelerated.
     */
    private SurfaceData bisd;

    public X11RemoteOffScreenImage(Component c,
                                   ColorModel cm, WritableRaster raster,
				   boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
        createNativeRaster();
    }

    protected SurfaceManager createSurfaceManager() {
        return new X11RemoteSurfaceManager(this);
    }

    private class X11RemoteSurfaceManager extends X11CachingSurfaceManager {

        public X11RemoteSurfaceManager(BufferedImage bi) {
            super(bi);

            if (!accelerationEnabled) {
                return;
            }

            GraphicsConfiguration gc = 
                X11SurfaceData.getGC(c == null ?
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

        protected void copyDefaultToAccelerated() {
            if (sdDefault != null && sdAccel != null &&
                sdDefault != sdAccel)
            {
                // we can't render directly from hw sd to hw sd
                // as we might end up in XCopyArea with pixmaps from different 
                // screens. So we use a temp BI SD as a bypass:
                // sdDefault -> bisd -> sdAccel
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
