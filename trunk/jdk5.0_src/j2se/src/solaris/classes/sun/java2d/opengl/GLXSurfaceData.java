/*
 * @(#)GLXSurfaceData.java	1.7 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    private static native void initIDs(Class glxgc);
    private native void initOps(X11ComponentPeer peer,
				GLXGraphicsConfig gc);
    protected native boolean initWindow(long pData);
    protected native boolean initPbuffer(long pCtx, long pData,
                                         int width, int height);
    protected native boolean initPixmap(long pCtx, long pData,
                                        int width, int height, int depth);

    static {
        initIDs(GLXGraphicsConfig.class);
    }

    protected GLXSurfaceData(X11ComponentPeer peer, GLXGraphicsConfig gc,
                             SurfaceType sType, ColorModel cm, int type)
    {
        super(sType, cm, type);
	this.peer = peer;
	this.graphicsConfig = gc;
	this.solidloops = graphicsConfig.getSolidLoops(sType);
	initOps(peer, graphicsConfig);
    }

    public boolean initWindow() {
        if (initWindow(getNativeOps())) {
            if (type == UNDEFINED) {
                type = WINDOW;
            }
            return true;
        }
        return false;
    }

    public GraphicsConfiguration getDeviceConfiguration() {
        return graphicsConfig;
    }

    public OGLContext getContext() {
        return graphicsConfig.getContext();
    }

    public long getSharedContext() {
        return graphicsConfig.getThreadSharedContext();
    }

    public boolean isBlendPremultAvailable() {
        return graphicsConfig.isBlendPremultAvailable();
    }

    public boolean isTexNonPow2Available() {
        return graphicsConfig.isTexNonPow2Available();
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
                                                     boolean forceback)
    {
        GLXGraphicsConfig gc = getGC(peer);
        Rectangle r = peer.getBounds();
        int type = (forceback ? OGLSurfaceData.FLIP_BACKBUFFER :
                                OGLSurfaceData.VOL_BACKBUFFER);
        // REMIND: what about image?
        return new GLXOffScreenSurfaceData(peer, gc, r.width, r.height,
                                           null, peer.getColorModel(), type);
    }

    /**
     * Creates a SurfaceData object representing an off-screen buffer (either
     * a Pixmap, Pbuffer, or Texture).
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
	    super(peer, gc, OpenGLSurface, peer.getColorModel(),
                  OGLSurfaceData.UNDEFINED);
	}

	public SurfaceData getReplacement() {
	    return peer.getSurfaceData();
	}

	public Rectangle getBounds() {
            Rectangle r = peer.getBounds();
            r.x = r.y = 0;
            return r;
	}
    }

    public static class GLXOffScreenSurfaceData extends GLXSurfaceData {

	private Image offscreenImage;
	private int width, height;
	private int transparency;

	public GLXOffScreenSurfaceData(X11ComponentPeer peer,
                                       GLXGraphicsConfig gc, 
                                       int width, int height,
                                       Image image, ColorModel cm,
                                       int type)
        {
	    super(peer, gc,
                  (type == TEXTURE) ? OpenGLTexture : OpenGLSurface,
                  cm, type);

            this.width = width;
            this.height = height;
            offscreenImage = image;
            transparency = cm.getTransparency();

            initSurface(width, height, depth);
	}

	public SurfaceData getReplacement() {
            return restoreContents(offscreenImage);
	}

	public int getTransparency() {
	    return transparency;
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
    }
}
