/*
 * @(#)GLXSurfaceData.java	1.17 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.X11ComponentPeer;
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;

public abstract class GLXSurfaceData extends OGLSurfaceData {

    protected X11ComponentPeer peer;
    private GLXGraphicsConfig graphicsConfig;

    private native void initOps(X11ComponentPeer peer, long aData);
    protected native boolean initPbuffer(long pData, long pConfigInfo,
                                         boolean isOpaque,
                                         int width, int height);

    protected GLXSurfaceData(X11ComponentPeer peer, GLXGraphicsConfig gc,
                             ColorModel cm, int type)
    {
        super(gc, cm, type);
	this.peer = peer;
	this.graphicsConfig = gc;
	initOps(peer, graphicsConfig.getAData());
    }

    public GraphicsConfiguration getDeviceConfiguration() {
        return graphicsConfig;
    }

    /**
     * Creates a SurfaceData object representing the primary (front) buffer
     * of an on-screen Window.
     */
    public static GLXWindowSurfaceData createData(X11ComponentPeer peer) {
        GLXGraphicsConfig gc = getGC(peer);
        return new GLXWindowSurfaceData(peer, gc);
    }

    /**
     * Creates a SurfaceData object representing the back buffer of a
     * double-buffered on-screen Window.
     */
    public static GLXOffScreenSurfaceData createData(X11ComponentPeer peer,
                                                     Image image,
                                                     int type)
    {
        GLXGraphicsConfig gc = getGC(peer);
        Rectangle r = peer.getBounds();
        if (type == FLIP_BACKBUFFER) {
            return new GLXOffScreenSurfaceData(peer, gc, r.width, r.height,
                                               image, peer.getColorModel(),
                                               FLIP_BACKBUFFER);
        } else {
            return new GLXVSyncOffScreenSurfaceData(peer, gc, r.width, r.height,
                                                    image, peer.getColorModel(),
                                                    type);
        }
    }

    /**
     * Creates a SurfaceData object representing an off-screen buffer (either
     * a Pbuffer or Texture).
     */
    public static GLXOffScreenSurfaceData createData(GLXGraphicsConfig gc,
                                                     int width, int height,
                                                     ColorModel cm, 
                                                     Image image, int type)
    {
	return new GLXOffScreenSurfaceData(null, gc, width, height,
                                           image, cm, type);
    }

    public static GLXGraphicsConfig getGC(X11ComponentPeer peer) {
	if (peer != null) {
	    return (GLXGraphicsConfig)peer.getGraphicsConfiguration();
	} else {
            // REMIND: this should rarely (never?) happen, but what if
            //         default config is not GLX?
	    GraphicsEnvironment env =
		GraphicsEnvironment.getLocalGraphicsEnvironment();
	    GraphicsDevice gd = env.getDefaultScreenDevice();
	    return (GLXGraphicsConfig)gd.getDefaultConfiguration();
	}
    }

    public static class GLXWindowSurfaceData extends GLXSurfaceData {

	public GLXWindowSurfaceData(X11ComponentPeer peer, 
                                    GLXGraphicsConfig gc)
        {
            super(peer, gc, peer.getColorModel(), WINDOW);
	}

	public SurfaceData getReplacement() {
	    return peer.getSurfaceData();
	}

	public Rectangle getBounds() {
            Rectangle r = peer.getBounds();
            r.x = r.y = 0;
            return r;
	}

	/**
	 * Returns destination Component associated with this SurfaceData.
	 */
	public Object getDestination() {
	    return peer.getTarget();
	}
    }

    /**
     * A surface which implements a v-synced flip back-buffer with COPIED
     * FlipContents.
     *
     * This surface serves as a back-buffer to the outside world, while
     * it is actually an offscreen surface. When the BufferStrategy this surface
     * belongs to is showed, it is first copied to the real private
     * FLIP_BACKBUFFER, which is then flipped.
     */
    public static class GLXVSyncOffScreenSurfaceData extends
        GLXOffScreenSurfaceData
    {
        private GLXOffScreenSurfaceData flipSurface;

	public GLXVSyncOffScreenSurfaceData(X11ComponentPeer peer,
                                            GLXGraphicsConfig gc,
                                            int width, int height,
                                            Image image, ColorModel cm,
                                            int type)
        {
            super(peer, gc, width, height, image, cm, type);
            flipSurface = GLXSurfaceData.createData(peer, image, FLIP_BACKBUFFER);
        }

        public SurfaceData getFlipSurface() {
            return flipSurface;
        }

        @Override
        public void flush() {
            flipSurface.flush();
            super.flush();
        }

    }

    public static class GLXOffScreenSurfaceData extends GLXSurfaceData {

	private Image offscreenImage;
	private int width, height;

	public GLXOffScreenSurfaceData(X11ComponentPeer peer,
                                       GLXGraphicsConfig gc, 
                                       int width, int height,
                                       Image image, ColorModel cm,
                                       int type)
        {
            super(peer, gc, cm, type);

            this.width = width;
            this.height = height;
            offscreenImage = image;

            initSurface(width, height);
	}

	public SurfaceData getReplacement() {
            return restoreContents(offscreenImage);
	}

	public Rectangle getBounds() {
            if (type == FLIP_BACKBUFFER) {
                Rectangle r = peer.getBounds();
                r.x = r.y = 0;
                return r;
            } else {
                return new Rectangle(width, height);
            }
	}
	
	/**
	 * Returns destination Image associated with this SurfaceData.
	 */
	public Object getDestination() {
	    return offscreenImage;
	}
    }
}
