/*
 * @(#)SunVolatileImage.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.ImageCapabilities;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.SurfaceManagerFactory;

/**
 * This class is the base implementation of the VolatileImage
 * abstract class.  The class implements most of the standard Image
 * methods (width, height, etc.) but delegates all surface management
 * issues to a platform-specific VolatileSurfaceManager.  When a new instance
 * of SunVolatileImage is created, it automatically creates an
 * appropriate VolatileSurfaceManager for the GraphicsConfiguration
 * under which this SunVolatileImage was created.
 */
public class SunVolatileImage extends VolatileImage implements Manageable {

    protected VolatileSurfaceManager surfaceManager;
    protected Component comp;
    private GraphicsConfiguration graphicsConfig;
    private Font defaultFont;
    private int width, height;

    private SunVolatileImage(Component comp,
                             GraphicsConfiguration graphicsConfig,
                             int width, int height, Object context,
			     int transparency)
    {
        this.comp = comp;
        this.graphicsConfig = graphicsConfig;
	this.width = width;
	this.height = height;
	this.transparency = transparency;
        this.surfaceManager = createSurfaceManager(context);

	// post-construction initialization of the surface manager
	surfaceManager.initialize();
        // clear the background
        surfaceManager.initContents();
    }

    private SunVolatileImage(Component comp,
                             GraphicsConfiguration graphicsConfig,
                             int width, int height, Object context)
    {
	this(comp, graphicsConfig,
             width, height, context, Transparency.OPAQUE);
    }

    public SunVolatileImage(Component comp, int width, int height) {
        this(comp, width, height, null);
    }

    public SunVolatileImage(Component comp,
                            int width, int height, Object context)
    {
	this(comp, comp.getGraphicsConfiguration(),
             width, height, context);
    }

    public SunVolatileImage(GraphicsConfiguration graphicsConfig, 
			    int width, int height)
    {
	this(null, graphicsConfig, width, height, null);
    }

    public SunVolatileImage(GraphicsConfiguration graphicsConfig, 
			    int width, int height, int transparency)
    {
	this(null, graphicsConfig, width, height, null, transparency);
    }

    public int getWidth() {
	return width;
    }

    public int getHeight() {
	return height;
    }

    public GraphicsConfiguration getGraphicsConfig() {
        return graphicsConfig;
    }

    public void updateGraphicsConfig() {
	// If this VImage is associated with a Component, get an updated
	// graphicsConfig from that component.  Otherwise, keep the one
	// that we were created with
	if (comp != null) {
	    GraphicsConfiguration gc = comp.getGraphicsConfiguration();
	    if (gc != null) {
		// Could potentially be null in some failure situations;
		// better to keep the old non-null value around than to
		// set graphicsConfig to null
		graphicsConfig = gc;
	    }
	}
    }
    
    public Component getComponent() {
        return comp;
    }

    public SurfaceManager getSurfaceManager() {
	return surfaceManager;
    }

    protected VolatileSurfaceManager createSurfaceManager(Object context) {
	/**
	 * Platform-specific SurfaceManagerFactories will return a
	 * manager suited to acceleration on each platform.  But if
	 * the user is asking for a VolatileImage from a BufferedImageGC,
	 * then we need to return the appropriate unaccelerated manager.
	 * Note: this could change in the future; if some platform would
	 * like to accelerate BIGC volatile images, then this special-casing
	 * of the BIGC graphicsConfig should live in platform-specific
	 * code instead.
	 */
	if (graphicsConfig instanceof BufferedImageGraphicsConfig) {
	    return new BufImgVolatileSurfaceManager(this, context);
	}
        return SurfaceManagerFactory.createVolatileManager(this, context);
    }

    private Color getForeground() {
	if (comp != null) {
	    return comp.getForeground();
	} else {
	    return Color.black;
	}
    }

    private Color getBackground() {
	if (comp != null) {
	    return comp.getBackground();
	} else {
	    return Color.white;
	}
    }

    private Font getFont() {
	if (comp != null) {
	    return comp.getFont();
	} else {
	    if (defaultFont == null) {
		defaultFont = new Font("Dialog", Font.PLAIN, 12);
	    }
	    return defaultFont;
	}
    }

    public Graphics2D createGraphics() {
	return new SunGraphics2D(surfaceManager.getDestSurfaceData(),
				 getForeground(),
				 getBackground(),
				 getFont());
    }

    // Image method implementations
    public Object getProperty(String name, ImageObserver observer) {
	if (name == null) {
	    throw new NullPointerException("null property name is not allowed");
	}
	return java.awt.Image.UndefinedProperty;
    }

    public int getWidth(ImageObserver observer) {
	return getWidth();
    }

    public int getHeight(ImageObserver observer) {
	return getHeight();
    }

    /**
     * This method creates a BufferedImage intended for use as a "snapshot"
     * or a backup surface.  It explicitly creates a BufferedImage instead
     * of simply calling graphicsConfig.createCompatibleImage() as the
     * latter method may return a RemoteOffScreenImage.  ROSI's are
     * problematic in this situation because certain code (in
     * BufImgSurfaceData and mediaLib) is unable to interpret the
     * WritableRasterNative contained within the ROSI.
     */
    public BufferedImage getBackupImage() {
        ColorModel cm = graphicsConfig.getColorModel(getTransparency());
        WritableRaster wr = cm.createCompatibleWritableRaster(getWidth(),
                                                              getHeight());
        return new BufferedImage(cm, wr, cm.isAlphaPremultiplied(), null);
    }

    public BufferedImage getSnapshot() {
        BufferedImage bi = getBackupImage();
        Graphics2D g = bi.createGraphics();
        g.setComposite(AlphaComposite.Src);
        g.drawImage(this, 0, 0, null);
        g.dispose();
        return bi;
    }

    public int validate(GraphicsConfiguration gc) {
        return surfaceManager.validate(gc);
    }
        
    public boolean contentsLost() {
	return surfaceManager.contentsLost();
    }

    public ImageCapabilities getCapabilities() {
        return surfaceManager.getCapabilities();
    }

    /**
     * Equivalent to the no-gc version of getCapabilities if the gc
     * is null or is the gc that the VolatileImage is associated with;
     * otherwise return the default (unaccelerated) caps.
     */
    public ImageCapabilities getCapabilities(GraphicsConfiguration gc) {
	if (surfaceManager.isConfigValid(gc)) {
	    return getCapabilities();
	}
	return super.getCapabilities(gc);
    }

    public void flush() {
        surfaceManager.flush();
    }
}
