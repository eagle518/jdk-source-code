/*
 * @(#)OffScreenImage.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Component;
import java.awt.Color;
import java.awt.SystemColor;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.image.ImageProducer;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.SurfaceManagerFactory;
import sun.java2d.loops.CompositeType;

/**
 * This is a special variant of BufferedImage that keeps a reference to
 * a Component.  The Component's foreground and background colors and
 * default font are used as the defaults for this image.
 */
public class OffScreenImage extends BufferedImage implements Manageable {

    protected Component c;
    protected SurfaceManager surfaceManager;
    private OffScreenImageSource osis;
    private Font defaultFont;

    /**
     * Constructs an OffScreenImage with the default image type,
     * TYPE_4BYTE_ABGR, for offscreen rendering to be used with a
     * given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     * REMIND:  At some point, we might want to look at the component
     * and figure out the best image type to construct.
     */
    public OffScreenImage(Component c, int width, int height) {
        // REMIND:  Should get image type from the component.getGC
        this(c, width, height, TYPE_4BYTE_ABGR);
    }

    /**
     * Constructs an OffScreenImage with the given image type,
     * for offscreen rendering to be used with a given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     */
    public OffScreenImage(Component c, int width, int height,
                          int imageType)
    {
        super(width, height, imageType);
        this.c = c;
	initSurfaceManager(width, height);
    }

    /**
     * Constructs an OffScreenImage given a color model and tile,
     * for offscreen rendering to be used with a given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     */
    public OffScreenImage(Component c, ColorModel cm, WritableRaster raster,
                          boolean isRasterPremultiplied)
    {
        super(cm, raster, isRasterPremultiplied, null);
        this.c = c;
	initSurfaceManager(raster.getWidth(), raster.getHeight());
    }

    public Graphics getGraphics() {
        return createGraphics();
    }

    public Graphics2D createGraphics() {
	if (c == null) {
            GraphicsEnvironment env =
                GraphicsEnvironment.getLocalGraphicsEnvironment();
            return env.createGraphics(this);
        }

        Color bg = c.getBackground();
        if (bg == null) {
            bg = SystemColor.window;
        }

        Color fg = c.getForeground();
        if (fg == null) {
            fg = SystemColor.windowText;
        }

        Font font = c.getFont();
        if (font == null) {
	    if (defaultFont == null) {
		defaultFont = new Font("Dialog", Font.PLAIN, 12);
	    }
	    font = defaultFont;
        }

        return new SunGraphics2D(surfaceManager.getDestSurfaceData(),
                                 fg, bg, font);
    }

    public SurfaceManager getSurfaceManager() {
        return surfaceManager;
    }

    protected SurfaceManager createSurfaceManager() {
        return SurfaceManagerFactory.createCachingManager(this);
    }
	        
    private void initSurfaceManager(int width, int height) {
        surfaceManager = createSurfaceManager();

        // clear background only if this image was created from a Component
        if (c == null) {
            return;
        }

	Graphics2D g2 = createGraphics();
	try {
	    g2.clearRect(0, 0, width, height);
	} finally {
	    g2.dispose();
	}
    }

    public ImageProducer getSource() {
	if (osis == null) {
	    osis = new OffScreenImageSource(this);
	}
        return osis;
    }
}
