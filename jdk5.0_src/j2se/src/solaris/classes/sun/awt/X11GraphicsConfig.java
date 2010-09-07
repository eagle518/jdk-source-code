/*
 * @(#)X11GraphicsConfig.java	1.66 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.AWTException;
import java.awt.BufferCapabilities;
import java.awt.Component;
import java.awt.Toolkit;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.Image;
import java.awt.ImageCapabilities;
import java.awt.image.DataBuffer;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.ImageProducer;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import java.awt.geom.AffineTransform;
import java.awt.Rectangle;
import sun.java2d.SurfaceData;
import sun.java2d.loops.RenderLoops;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.CompositeType;
import sun.awt.image.OffScreenImage;
import sun.awt.image.SunVolatileImage;
import sun.awt.X11ComponentPeer;
import sun.awt.motif.X11RemoteOffScreenImage;

/**
 * This is an implementation of a GraphicsConfiguration object for a
 * single X11 visual.
 *
 * @see GraphicsEnvironment
 * @see GraphicsDevice
 * @version 1.66, 12/19/03
 */
public class X11GraphicsConfig extends GraphicsConfiguration {
    protected X11GraphicsDevice screen;
    protected int visual;
    int depth;
    int colormap;
    ColorModel colorModel;
    long aData;
    boolean doubleBuffer;
    private BufferCapabilities bufferCaps;
    private static ImageCapabilities imageCaps = new ImageCapabilities(true);

    // will be set on native level from init()
    protected int bitsPerPixel;

    protected SurfaceType surfaceType;
    
    public RenderLoops solidloops;

    public static X11GraphicsConfig getConfig(X11GraphicsDevice device,
                                              int visualnum, int depth,
                                              int colormap,
                                              boolean doubleBuffer)
    {
	return new X11GraphicsConfig(device, visualnum, depth, colormap, doubleBuffer);
    }

    /*
     * Note this method is currently here for backward compatability
     * as this was the method used in jdk 1.2 beta4 to create the
     * X11GraphicsConfig objects. Java3D code had called this method
     * explicitly so without this, if a user tries to use JDK1.2 fcs
     * with Java3D beta1, a NoSuchMethod execption is thrown and
     * the program exits. REMOVE this method after Java3D fcs is
     * released!
     */
    public static X11GraphicsConfig getConfig(X11GraphicsDevice device,
					      int visualnum, int depth,
                                              int colormap, int type)
    {
	return new X11GraphicsConfig(device, visualnum, depth, colormap, false);
    }

    private native int getNumColors();
    private native void init(int visualNum, int screen);
    private native ColorModel makeColorModel();
    
    protected X11GraphicsConfig(X11GraphicsDevice device,
			        int visualnum, int depth,
                                int colormap, boolean doubleBuffer)
    {
	this.screen = device;
	this.visual = visualnum;
        this.doubleBuffer = doubleBuffer;
        this.depth = depth;
        this.colormap = colormap;
	init (visualnum, screen.getScreen());
    }    

    /**
     * Return the graphics device associated with this configuration.
     */
    public GraphicsDevice getDevice() {
	return screen;
    }

    /**
     * Returns the visual id associated with this configuration.
     */
    public int getVisual () {
	return visual;
    }


    /**
     * Returns the depth associated with this configuration.
     */
    public int getDepth () {
	return depth;
    }

    /**
     * Returns the colormap associated with this configuration.
     */
    public int getColormap () {
	return colormap;
    }

    /**
     * Returns a number of bits allocated per pixel 
     * (might be different from depth)
     */
    public int getBitsPerPixel() {
	return bitsPerPixel;
    }

    public synchronized SurfaceType getSurfaceType() {
	if (surfaceType != null) {
	    return surfaceType;
	}

	surfaceType = X11SurfaceData.getSurfaceType(this, false);
	return surfaceType;
    }

    /**
     * Return the RenderLoops this type of destination uses for
     * solid fills and strokes.
     */
    public synchronized RenderLoops getSolidLoops(SurfaceType stype) {
	if (solidloops == null) {
	    solidloops = SurfaceData.makeRenderLoops(SurfaceType.OpaqueColor,
						     CompositeType.SrcNoEa,
						     stype);
	}
	return solidloops;
    }

    /**
     * Returns a BufferedImage with channel layout and color model
     * compatible with this graphics configuration.  This method
     * has nothing to do with memory-mapping
     * a device.  This BufferedImage has
     * a layout and color model
     * that is closest to this native device configuration and thus
     * can be optimally blitted to this device.
     */
    public BufferedImage createCompatibleImage(int width, int height) {
	ColorModel model = getColorModel();
	WritableRaster raster =
            model.createCompatibleWritableRaster(width, height);
	if (X11SurfaceData.isAccelerationEnabled()) {
	    return new X11RemoteOffScreenImage(null, model, raster,
                                               model.isAlphaPremultiplied());
	}
        return new BufferedImage(model, raster,
                                 model.isAlphaPremultiplied(), null);
    }

    /**
     * Returns a VolatileImage
     * compatible with this graphics configuration.
     * The returned <code>VolatileImage</code> has
     * a layout and color model that is closest to this native device
     * and may have data that is stored on 
     * the device (i.e., in a pixmap) and can therefore be rendered to and 
     * blitted from using platform-specific acceleration.
     */
    public VolatileImage createCompatibleVolatileImage(int width, int height) {
	return new SunVolatileImage(this, width, height);
    }

    public VolatileImage createCompatibleVolatileImage(int width, int height,
						       int transparency)
    {
	return new SunVolatileImage(this, width, height, transparency);
    }

    /**
     * Returns a BufferedImage that supports the specified transparency
     * and has a channel layout and color model
     * compatible with this graphics configuration.  This method
     * has nothing to do with memory-mapping
     * a device. This BufferedImage has a layout and
     * color model that can be optimally blitted to a device
     * with this configuration.  
     * @see Transparency#OPAQUE
     * @see Transparency#BITMASK
     * @see Transparency#TRANSLUCENT
     */
    public BufferedImage createCompatibleImage(int width, int height,
					       int transparency) {

        switch (transparency) {
        case Transparency.OPAQUE:
	    return createCompatibleImage(width, height);

        case Transparency.BITMASK:
        case Transparency.TRANSLUCENT:
            ColorModel cm = getColorModel(transparency);
	    WritableRaster wr =
                cm.createCompatibleWritableRaster(width, height);
            return new BufferedImage(cm, wr,
                                     cm.isAlphaPremultiplied(), null);

        default:
            throw new IllegalArgumentException("Unknown transparency type "+
                                               transparency);
        }
    }

    /**
     * Returns the color model associated with this configuration.
     */
    public synchronized ColorModel getColorModel() {
	if (colorModel == null)  {
	    // Force SystemColors to be resolved before we create the CM
	    java.awt.SystemColor.window.getRGB();
            // This method, makeColorModel(), can return null if the
            // toolkit is not initialized yet.
            // The toolkit will then call back to this routine after it
            // is initialized and makeColorModel() should return a non-null
            // colorModel.
	    colorModel = makeColorModel();
	    if (colorModel == null)
		colorModel = Toolkit.getDefaultToolkit ().getColorModel ();
	}
	
	return colorModel;
    }

    /**
     * Returns the color model associated with this configuration that
     * supports the specified transparency.
     */
    public ColorModel getColorModel(int transparency) {
	switch (transparency) {
	case Transparency.OPAQUE:
            return getColorModel();
	case Transparency.BITMASK:
            return new DirectColorModel(25, 0xff0000, 0xff00, 0xff, 0x1000000);
	case Transparency.TRANSLUCENT:
            return ColorModel.getRGBdefault();
	default:
	    return null;
        }
    }

    /**
     * Returns the default Transform for this configuration.  This
     * Transform is typically the Identity transform for most normal
     * screens.  Device coordinates for screen and printer devices will
     * have the origin in the upper left-hand corner of the target region of
     * the device, with X coordinates
     * increasing to the right and Y coordinates increasing downwards.
     * For image buffers, this Transform will be the Identity transform.
     */
    public AffineTransform getDefaultTransform() {
	return new AffineTransform();
    }
    
    /**
     *
     * Returns a Transform that can be composed with the default Transform
     * of a Graphics2D so that 72 units in user space will equal 1 inch
     * in device space.  
     * Given a Graphics2D, g, one can reset the transformation to create
     * such a mapping by using the following pseudocode:
     * <pre>
     *      GraphicsConfiguration gc = g.getGraphicsConfiguration();
     *
     *      g.setTransform(gc.getDefaultTransform());
     *      g.transform(gc.getNormalizingTransform());
     * </pre>
     * Note that sometimes this Transform will be identity (e.g. for 
     * printers or metafile output) and that this Transform is only
     * as accurate as the information supplied by the underlying system.
     * For image buffers, this Transform will be the Identity transform,
     * since there is no valid distance measurement.
     */
    public AffineTransform getNormalizingTransform() {
	double xscale = getXResolution(screen.getScreen()) / 72.0;
	double yscale = getYResolution(screen.getScreen()) / 72.0;
	return new AffineTransform(xscale, 0.0, 0.0, yscale, 0.0, 0.0);
    }

    private native double getXResolution(int screen);
    private native double getYResolution(int screen);

    public long getAData() {
        return aData;
    }

    public String toString() {
	return ("X11GraphicsConfig[dev="+screen+
		",vis=0x"+Integer.toHexString(visual)+
		"]");
    }

    /*
     * Initialize JNI field and method IDs for fields that may be
     *  accessed from C.
     */  
    private static native void initIDs();

    static {
	initIDs ();
    }

    public Rectangle getBounds() {
        return pGetBounds(screen.getScreen());
    }

    public native Rectangle pGetBounds(int screenNum);

    // XDBECapabilities can be reused for GLXCapabilities    
    private static class XDBECapabilities extends BufferCapabilities {
        public XDBECapabilities() {
            super(imageCaps, imageCaps, FlipContents.UNDEFINED);
        }
    }
    
    public BufferCapabilities getBufferCapabilities() {
        if (bufferCaps == null) {
            if (doubleBuffer) {
                bufferCaps = new XDBECapabilities();
            } else {
                bufferCaps = super.getBufferCapabilities();
            }
        }
        return bufferCaps;
    }
    
    public ImageCapabilities getImageCapabilities() {
        return imageCaps;
    }

    public boolean isDoubleBuffered() {
        return doubleBuffer;
    }

    /**
     * The following methods are invoked from {M,X}Toolkit.java and
     * X11ComponentPeer.java rather than having the X11-dependent
     * implementations hardcoded in those classes.  This way the appropriate
     * actions are taken based on the peer's GraphicsConfig, whether it is
     * an X11GraphicsConfig or a GLXGraphicsConfig.
     */

    /**
     * Creates a new SurfaceData that will be associated with the given
     * X11ComponentPeer.
     */
    public SurfaceData createSurfaceData(X11ComponentPeer peer) {
        return X11SurfaceData.createData(peer);
    }

    /**
     * Creates a new hidden-acceleration image of the given width and height
     * that is associated with the target Component.
     */
    public Image createAcceleratedImage(Component target,
                                        int width, int height)
    {
        ColorModel model = getColorModel(Transparency.OPAQUE);
        WritableRaster wr = 
            model.createCompatibleWritableRaster(width, height);
        if (X11SurfaceData.isAccelerationEnabled()) {
            return new X11RemoteOffScreenImage(target, model, wr,
                                               model.isAlphaPremultiplied());
        }
        return new OffScreenImage(target, model, wr,
                                  model.isAlphaPremultiplied());
    }

    /**
     * The following methods correspond to the multibuffering methods in
     * X11ComponentPeer.java...
     */

    private native long createBackBuffer(long window, int swapAction);
    private native void swapBuffers(long window, int swapAction);

    /**
     * Attempts to create an XDBE-based backbuffer for the given peer.  If
     * the requested configuration is not natively supported, an AWTException
     * is thrown.  Otherwise, if the backbuffer creation is successful, a
     * handle to the native backbuffer is returned.
     */
    public long createBackBuffer(X11ComponentPeer peer,
                                 int numBuffers, BufferCapabilities caps)
        throws AWTException
    {
        if (!X11GraphicsDevice.isDBESupported()) {
            throw new AWTException("Page flipping is not supported");
        }
        if (numBuffers > 2) {
            throw new AWTException(
                "Only double or single buffering is supported");
        }
        BufferCapabilities configCaps = getBufferCapabilities();
        if (!configCaps.isPageFlipping()) {
            throw new AWTException("Page flipping is not supported");
        }

        long window = peer.getContentWindow();
        int swapAction = getSwapAction(caps.getFlipContents());

        return createBackBuffer(window, swapAction);
    }

    /**
     * Destroys the backbuffer object represented by the given handle value.
     */
    public native void destroyBackBuffer(long backBuffer);

    /**
     * Creates a VolatileImage that essentially wraps the target Component's
     * backbuffer, using the provided backbuffer handle.
     */
    public VolatileImage createBackBufferImage(Component target,
                                               long backBuffer)
    {
        return new SunVolatileImage(target,
                                    target.getWidth(), target.getHeight(),
                                    new Long(backBuffer));
    }

    /**
     * Performs the native XDBE flip operation for the given target Component.
     */
    public void flip(X11ComponentPeer peer,
                     Component target, VolatileImage xBackBuffer,
                     BufferCapabilities.FlipContents flipAction)
    {
        long window = peer.getContentWindow();
        int swapAction = getSwapAction(flipAction);
        swapBuffers(window, swapAction);
    }
    
    /**
     * Maps the given FlipContents constant to the associated XDBE swap
     * action constant.
     */
    private static int getSwapAction(
        BufferCapabilities.FlipContents flipAction) {
        if (flipAction == BufferCapabilities.FlipContents.BACKGROUND) {
            return 0x01;
        } else if (flipAction == BufferCapabilities.FlipContents.PRIOR) {
            return 0x02;
        } else if (flipAction == BufferCapabilities.FlipContents.COPIED) {
            return 0x03;
        } else {
            return 0x00; // UNDEFINED
        }
    }
}
