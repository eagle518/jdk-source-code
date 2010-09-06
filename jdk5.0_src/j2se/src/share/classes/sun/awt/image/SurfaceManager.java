/*
 * @(#)SurfaceManager.java	1.4 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.ImageCapabilities;
import java.awt.image.BufferedImage;
import sun.java2d.SurfaceData;
import sun.java2d.SurfaceManagerFactory;
import sun.java2d.loops.CompositeType;

/**
 * The abstract base class that manages the various SurfaceData objects that
 * represent an Image's contents.  Subclasses can customize how the surfaces
 * are organized, whether to cache the original contents in an accelerated
 * surface, and so on.
 */
public abstract class SurfaceManager {

    private static native void initIDs();
    private static native SurfaceManager
        getSurfaceManager(BufferedImage bi);
    private static native void
        setSurfaceManager(BufferedImage bi, SurfaceManager sm);

    static {
	initIDs();
    }

    /**
     * Returns the SurfaceManager object contained within the given Image.
     */
    public static SurfaceManager getManager(Image img) {
	if (img instanceof Manageable) {
            return ((Manageable)img).getSurfaceManager();
        } else {
            try {
                return getManager((BufferedImage)img);
            } catch (ClassCastException e) {
                throw new IllegalArgumentException("Invalid Image variant");
            }
        }
    }

    /**
     * Returns the SurfaceManager object from the given BufferedImage.  The
     * surfaceManager instance variable is package protected in BufferedImage,
     * so it must be fetched from JNI.  If the SurfaceManager has not been
     * initialized for the supplied BufferedImage, a new CachingSurfaceManager
     * is created and is implanted into the BufferedImage.
     */
    private static SurfaceManager getManager(BufferedImage bi) {
	if (bi == null) {
	    throw new NullPointerException("BufferedImage cannot be null");
	}

        SurfaceManager sMgr = getSurfaceManager(bi);
        if (sMgr != null) {
            return sMgr;
        }

        sMgr = SurfaceManagerFactory.createCachingManager(bi);
	setSurfaceManager(bi, sMgr);
        return sMgr;
    }

    /**
     * The ImageCapabilities that reflect the current accelerated state
     * of this SurfaceManager's various surfaces.  Subclasses can cache
     * their own ImageCapabilities variant here.
     * REMIND: we probably do not need (or want) this protected
     * variable; subclasses should override getCapabilites() and
     * therefore will not need this variable in this superclass.
     */
    protected ImageCapabilities imageCaps;

    /**
     * Returns the best SurfaceData object to be used as the source surface
     * in an image copy operation.  The supplied parameters should describe
     * the type of operation being performed, so that an appropriate surface
     * can be used in the operation.  For example, if the destination surface
     * is "accelerated", this method should return a surface that is most
     * compatible with the destination surface.
     */
    public abstract SurfaceData getSourceSurfaceData(SurfaceData dstData,
                                                     CompositeType comp,
                                                     Color bgColor, 
                                                     boolean scale);

    /**
     * Returns the best SurfaceData object to be used as the destination
     * surface in a rendering operation.
     */
    public abstract SurfaceData getDestSurfaceData();

    /**
     * Restores the primary surface being managed, and then returns the
     * replacement surface.  This is called when an accelerated surface has
     * been "lost", in an attempt to auto-restore its contents.
     */
    public abstract SurfaceData restoreContents();

    /**
     * Notification that any accelerated surfaces associated with this manager
     * have been "lost", which might mean that they need to be manually
     * restored or recreated.
     * 
     * The default implementation does nothing, but platform-specific 
     * variants which have accelerated surfaces should perform any necessary
     * actions.
     */
    public void acceleratedSurfaceLost() {}

    /**
     * Returns an ImageCapabilities object which can be
     * inquired as to the specific capabilities of this
     * Image.  This default implementation returns an unaccelerated
     * ImageCapabilities object.  It is expected that sublcasses which
     * accelerate their images will override this method or change the
     * value of the imageCaps variable appropriately.
     *
     * @see java.awt.Image#getCapabilities
     */
    public ImageCapabilities getCapabilities(GraphicsConfiguration gc) {
	if (imageCaps == null) {
	    imageCaps = new ImageCapabilities(false);
	}
	return imageCaps;
    }

    /**
     * Releases system resources in use by ancillary SurfaceData objects,
     * such as surfaces cached in accelerated memory.  For example, a
     * CachingSurfaceManager should release all of its cached surfaces,
     * but the base system memory surface will not be affected.
     *
     * The default implementation does nothing, but platform-
     * specific variants should free native surfaces, such as texture objects
     * being cached in VRAM.
     */
    public void flush() {
    }
}
