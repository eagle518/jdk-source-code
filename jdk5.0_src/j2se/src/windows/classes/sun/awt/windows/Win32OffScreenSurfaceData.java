/*
 * @(#)Win32OffScreenSurfaceData.java	1.52 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Color;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;

import sun.awt.SunHints;
import sun.awt.WindowsFlags;
import sun.awt.Win32ColorModel24;
import sun.awt.Win32GraphicsConfig;
import sun.awt.Win32GraphicsDevice;
import sun.awt.image.SurfaceManager;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.WritableRasterNative;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.pipe.PixelToShapeConverter;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.RenderLoops;

/**
 * Win32OffScreenSurfaceData
 * 
 * This class implements a hardware-accelerated video memory surface.  It uses
 * a custom renderer (Win32DDRenderer) to render via DirectDraw into the 
 * surface and uses a custom Blit loop (Win32BlitLoops) to copy between
 * two hardware-accelerated surfaces (including the screen).
 */
public class Win32OffScreenSurfaceData extends SurfaceData {

    private int width;
    private int height;
    private int transparency;

    private GraphicsConfiguration graphicsConfig;
    private Image image;
    private RenderLoops solidloops;
    private boolean localD3dEnabled = true;
    protected boolean d3dClippingEnabled = false;
    private boolean ddSurfacePunted = false;

    private static native void initIDs();

    static {
	initIDs();
	// REMIND: This isn't really thought-out; if the user doesn't have or
	// doesn't want ddraw then we should not even have this surface type
	// in the loop
        if (WindowsFlags.isDDEnabled() && WindowsFlags.isDDOffscreenEnabled()) {
	    if (WindowsFlags.isDDBlitEnabled()) {
		// Register out hardware-accelerated Blit loops
		Win32BlitLoops.register();
	    }
	    if (WindowsFlags.isDDScaleEnabled()) {
		Win32ScaleLoops.register();
	    }
	    if (WindowsFlags.isTranslucentAccelerationEnabled()) {
		D3DBlitLoops.register();
	    }
	}
    }

    public static SurfaceType getSurfaceType(ColorModel cm, int transparency) {
	/**
	 * REMIND: If ddraw not available, set sType to non-ddraw surface type
	 * REMIND: If d3d not available, set sType to non-d3d surface type.
	 * Note that dd/d3d availability should be device-based, not just
	 * the global flags.
	 */
	if (transparency == Transparency.TRANSLUCENT) {
	    if (cm.getPixelSize() == 16) {
		return Win32SurfaceData.Ushort4444ArgbD3D;
	    } else {
		return Win32SurfaceData.IntArgbD3D;
	    }
	}
	boolean transparent = (transparency == Transparency.BITMASK);
	switch (cm.getPixelSize()) {
	case 32:
	case 24:
	    if (cm instanceof DirectColorModel) {
		if (((DirectColorModel)cm).getRedMask() == 0xff0000) {
		    return transparent ? Win32SurfaceData.IntRgbDD_BM : 
					 Win32SurfaceData.IntRgbD3D;
		} else {
		    return transparent ? Win32SurfaceData.IntRgbxDD_BM : 
					 Win32SurfaceData.IntRgbxD3D;
		}
	    } else {
		return transparent ? Win32SurfaceData.ThreeByteBgrDD_BM : 
				     Win32SurfaceData.ThreeByteBgrD3D;
	    }
	case 15:
	    return transparent ? Win32SurfaceData.Ushort555RgbDD_BM : 
				 Win32SurfaceData.Ushort555RgbD3D;
	case 16:
    	    if ((cm instanceof DirectColorModel) &&
		(((DirectColorModel)cm).getBlueMask() == 0x3e)) 
	    {
		return transparent ? Win32SurfaceData.Ushort555RgbxDD_BM : 
				     Win32SurfaceData.Ushort555RgbxD3D;
	    } else {
		return transparent ? Win32SurfaceData.Ushort565RgbDD_BM : 
				     Win32SurfaceData.Ushort565RgbD3D;
	    }
	case 8:
	    if (cm.getColorSpace().getType() == ColorSpace.TYPE_GRAY &&
		cm instanceof ComponentColorModel) {
		return transparent ? Win32SurfaceData.ByteGrayDD_BM : 
				     Win32SurfaceData.ByteGrayDD;
	    } else if (cm instanceof IndexColorModel &&
		       isOpaqueGray((IndexColorModel)cm)) {
		return transparent ? Win32SurfaceData.Index8GrayDD_BM : 
				     Win32SurfaceData.Index8GrayDD;
	    } else {
		return transparent ? Win32SurfaceData.ByteIndexedDD_BM : 
				     Win32SurfaceData.ByteIndexedOpaqueDD;
	    }
	default:
	    throw new sun.java2d.InvalidPipeException("Unsupported bit " +
						      "depth: " + 
						      cm.getPixelSize());
	}
    }

    public static Win32OffScreenSurfaceData
        createData(int width, int height,
                   ColorModel cm, GraphicsConfiguration gc,
                   Image image, int transparency)
    {
        Win32GraphicsDevice gd = (Win32GraphicsDevice)gc.getDevice();
        if (!gd.isOffscreenAccelerationEnabled() ||
	    ((transparency == Transparency.TRANSLUCENT) &&
	     !gd.isD3DEnabledOnDevice()))
	{
            // If acceleration for this type of image is disabled on this 
	    // device, do not create an accelerated surface type
            return null;
        }

        return new Win32OffScreenSurfaceData(width, height,
                                             getSurfaceType(cm, transparency),
                                             cm, gc, image, transparency,
                                             gd.getScreen());
    }

    protected static Win32D3DRenderer d3dNoClipPipe;
    protected static Win32D3DRenderer d3dClipPipe;
    protected static Win32DDRenderer ddPipe;
    protected static PixelToShapeConverter d3dTxNoClipPipe;
    protected static PixelToShapeConverter d3dTxClipPipe;
    protected static PixelToShapeConverter ddTxPipe;
    // The next 2 instance variables are set if d3dEnabled during
    // construction, depending on whether d3d can handle clipping
    // or not
    protected Win32D3DRenderer d3dPipe = null;
    protected PixelToShapeConverter d3dTxPipe = null;
    
    static {
	d3dNoClipPipe = new Win32D3DRenderer(false);
	d3dClipPipe = new Win32D3DRenderer(true);
	ddPipe = new Win32DDRenderer();
	if (GraphicsPrimitive.tracingEnabled()) {
	    d3dNoClipPipe = d3dNoClipPipe.traceWrapD3D();
	    d3dClipPipe = d3dClipPipe.traceWrapD3D();
	    ddPipe = ddPipe.traceWrapDD();
	}
	d3dTxNoClipPipe = new PixelToShapeConverter(d3dNoClipPipe);
	d3dTxClipPipe = new PixelToShapeConverter(d3dClipPipe);
	ddTxPipe = new PixelToShapeConverter(ddPipe);
    }

    public void validatePipe(SunGraphics2D sg2d) {
	if (sg2d.antialiasHint != SunHints.INTVAL_ANTIALIAS_ON &&
	    sg2d.paintState == sg2d.PAINT_SOLIDCOLOR &&
            sg2d.compositeState == sg2d.COMP_ISCOPY &&
	    sg2d.clipState != sg2d.CLIP_SHAPE &&
	    transparency != Transparency.TRANSLUCENT)
	                 // translucent images are accelerated via textures on
			 // Windows.  We cannot use dd/d3d/gdi to render to
			 // textures, so we must fallback to the software loops.
	{
            PixelToShapeConverter txPipe;
            Win32DDRenderer nontxPipe;
            if (WindowsFlags.isD3DEnabled() && localD3dEnabled) {
                txPipe    = d3dTxPipe;
                nontxPipe = d3dPipe;
            } else {
                txPipe    = ddTxPipe;
                nontxPipe = ddPipe;
            }
	    sg2d.imagepipe = imagepipe;
	    if (sg2d.transformState >= sg2d.TRANSFORM_TRANSLATESCALE) {
		sg2d.drawpipe = txPipe;
		sg2d.fillpipe = txPipe;
	    } else if (sg2d.strokeState != sg2d.STROKE_THIN){
		sg2d.drawpipe = txPipe;
		sg2d.fillpipe = nontxPipe;
	    } else {
		sg2d.drawpipe = nontxPipe;
		sg2d.fillpipe = nontxPipe;
	    }
	    sg2d.shapepipe = nontxPipe;
            if (sg2d.textAntialiasHint == SunHints.INTVAL_TEXT_ANTIALIAS_ON) {
                sg2d.textpipe = aaTextRenderer;
            } else {
                sg2d.textpipe = solidTextRenderer;
            }
	    // This is needed for AA text.
	    // Note that even a SolidTextRenderer can dispatch AA text
	    // if a GlyphVector overrides the AA setting.
	    sg2d.loops = solidloops;
	} else {
	    super.validatePipe(sg2d);
	}
    }

    /**
     * Disables D3D on this surfaceData object.  This can happen
     * when we encounter an error in rendering a D3D primitive
     * (for example, if we were unable to create a D3D device).
     * Upon next validation, this renderer will then choose a
     * non-D3D pipe.
     */
    public void disableD3D() {
	localD3dEnabled = false;
    }

    public static boolean isDDScaleEnabled() {
	return WindowsFlags.isDDScaleEnabled();
    }

    private WritableRasterNative wrn = null;
    public synchronized Raster getRaster(int x, int y, int w, int h) {
	if (wrn == null) {
	    wrn = WritableRasterNative.createNativeRaster(getColorModel(), 
							  this,
							  width, height);
	    if (wrn == null) {
		throw new InternalError("Unable to create native raster");
	    }
	}

	return wrn;
    }

    public void lock() {
	// REMIND: Do we need this call here?  Who calls the Java method?
    }

    public void unlock() {
	// REMIND: Do we need this call here?  Who calls the Java method?
    }

    public RenderLoops getRenderLoops(SunGraphics2D sg2d) {
	if (sg2d.paintState == sg2d.PAINT_SOLIDCOLOR &&
	    sg2d.compositeState == sg2d.COMP_ISCOPY)
	{
	    return solidloops;
	}
	return super.getRenderLoops(sg2d);
    }

    public GraphicsConfiguration getDeviceConfiguration() {
	return graphicsConfig;
    }

    /**
     * Initializes the native Ops pointer.
     */
    private native void initOps(int depth, int transparency);

    /**
     * This native method creates the offscreen surface in video memory and
     * (if necessary) initializes DirectDraw
     */
    private native void initSurface(int depth, int width, int height, 
                                    int screen, 
                                    boolean isVolatile,
				    int transparency);

    protected void initD3DPipes() {
        // d3dClippingEnabled is set during the call to initSurface
        if (d3dClippingEnabled) {
	    d3dPipe = d3dClipPipe;
	    d3dTxPipe = d3dTxClipPipe;
	} else {
	    d3dPipe = d3dNoClipPipe;
	    d3dTxPipe = d3dTxNoClipPipe;
	}
    }

    public native void restoreSurface();

    /**
     * Protected constructor (shared with Win32BackBufferSurfaceData).
     */
    protected Win32OffScreenSurfaceData(int width, int height,
                                        SurfaceType sType, ColorModel cm,
                                        GraphicsConfiguration gc,
                                        Image image, int transparency)
    {
	super(sType, cm);
	this.width = width;
	this.height = height;
	this.graphicsConfig = gc;
	this.image = image;
	this.transparency = transparency;
	this.solidloops =
	    ((Win32GraphicsConfig)graphicsConfig).getSolidLoops(sType);
        initOps(cm.getPixelSize(), transparency);
    }

    /**
     * Private constructor.  Use createData() to create an object.
     */
    private Win32OffScreenSurfaceData(int width, int height,
                                      SurfaceType sType, ColorModel cm,
                                      GraphicsConfiguration gc,
                                      Image image, int transparency,
                                      int screen)
    {
        this(width, height, sType, cm, gc, image, transparency);
        initSurface(cm.getPixelSize(), width, height, screen,
                    (image instanceof SunVolatileImage), transparency);
        initD3DPipes();
    }    

    /**
     * Need this since the surface data is created with
     * the color model of the target GC, which is always
     * opaque. But in SunGraphics2D.blitSD we choose loops
     * based on the transparency on the source SD, so 
     * we could choose wrong loop (blit instead of blitbg,
     * for example, which will cause problems in transparent
     * case).
     */
    public int getTransparency() {
	return transparency;
    }

    /**
     * When someone asks for a new surface data, we punt to our
     * container image which will attempt to restore the contents
     * of this surface or, failing that, will return null.
     */
    public SurfaceData getReplacement() {
        return restoreContents(image);
    }

    public Rectangle getBounds() {
	return new Rectangle(width, height);
    }
    
    private native void nativeInvalidate();

    public void invalidate() {
        if (isValid()) {
	    synchronized (this) {
		wrn = null;
	    }
            nativeInvalidate();
            super.invalidate();
        }
    }

    public native void setTransparentPixel(int pixel);

    public native void flush();

    /**
     * Returns true if the native representation of this image has been
     * moved into ddraw system memory.  This happens when many reads
     * or read-modify-write operations are requested of that surface.
     * If we have moved that surface into system memory, we should note that
     * here so that someone wanting to copy something to this surface will
     * take that into account during that copy.
     */
    public boolean surfacePunted() {
	return ddSurfacePunted;
    }
    
    protected void markSurfaceLost() {
	synchronized (this) {
	    wrn = null;
	}
	setSurfaceLost(true);
	SurfaceManager sMgr = SurfaceManager.getManager(image);
	sMgr.acceleratedSurfaceLost();
    }
}
