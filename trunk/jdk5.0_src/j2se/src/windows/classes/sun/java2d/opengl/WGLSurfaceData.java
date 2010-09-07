/*
 * @(#)WGLSurfaceData.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.Component;
import java.awt.Container;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.windows.WComponentPeer;
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;

public abstract class WGLSurfaceData extends OGLSurfaceData {

    protected WComponentPeer peer;
    private WGLGraphicsConfig graphicsConfig;

    private static native void initIDs(Class wglgc, Object lock);
    private native void initOps(WComponentPeer peer,
				WGLGraphicsConfig gc);
    protected native boolean initWindow(long pData, long pPeerData,
                                        long pConfigData,
                                        int xoff, int yoff);
    protected native boolean initPbuffer(long pCtx, long pData,
                                         int width, int height);

    static {
        initIDs(WGLGraphicsConfig.class, OGLContext.LOCK);
    }

    protected WGLSurfaceData(WComponentPeer peer, WGLGraphicsConfig gc,
                             SurfaceType sType, ColorModel cm, int type)
    {
        super(sType, cm, type);
	this.peer = peer;
	this.graphicsConfig = gc;
	this.solidloops = graphicsConfig.getSolidLoops(sType);
	initOps(peer, graphicsConfig);
    }

    public boolean initWindow() {
        int xoff, yoff;
        Component c = (Component)peer.getTarget();
        if (c instanceof Container) {
            Insets insets = ((Container)c).getInsets();
            if (type == VOL_BACKBUFFER) {
                xoff = 0;
                yoff = -insets.top;
            } else {
                xoff = -insets.left;
                yoff = -insets.bottom;
            }
        } else {
            xoff = yoff = 0;
        }

        long peerData = peer.getData();
        long configData = graphicsConfig.getNativeConfigInfo();
        if (initWindow(getNativeOps(), peerData, configData, xoff, yoff)) {
            if (type == UNDEFINED) {
                type = WINDOW;
            }
            return true;
        }

        return false;
    }

    protected boolean initPixmap(long pCtx, long pData,
                                 int width, int height, int depth)
    {
        // pixmaps not supported in WGL...
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
    public static WGLWindowSurfaceData createData(WComponentPeer peer) {
        WGLGraphicsConfig gc = getGC(peer);
        return new WGLWindowSurfaceData(peer, gc);
    }

    /**
     * Creates a SurfaceData object representing the back buffer of a
     * double-buffered on-screen Window.
     */
    public static WGLOffScreenSurfaceData createData(WComponentPeer peer,
                                                     boolean forceback)
    {
        WGLGraphicsConfig gc = getGC(peer);
        Rectangle r = peer.getBounds();
        int type = (forceback ? OGLSurfaceData.FLIP_BACKBUFFER :
                                OGLSurfaceData.VOL_BACKBUFFER);
        // REMIND: what about image?
        return new WGLOffScreenSurfaceData(peer, gc, r.width, r.height,
                                           null, peer.getColorModel(), type);
    }

    /**
     * Creates a SurfaceData object representing an off-screen buffer (either
     * a Pbuffer or Texture).
     */
    public static WGLOffScreenSurfaceData createData(WGLGraphicsConfig gc,
                                                     int width, int height,
                                                     ColorModel cm, 
                                                     Image image, int type)
    {
	return new WGLOffScreenSurfaceData(null, gc, width, height,
                                           image, cm, type);
    }

    public static WGLGraphicsConfig getGC(WComponentPeer peer) {
	if (peer != null) {
	    return (WGLGraphicsConfig)peer.getGraphicsConfiguration();
	} else {
            // REMIND: this should rarely (never?) happen, but what if
            //         default config is not WGL?
	    GraphicsEnvironment env =
		GraphicsEnvironment.getLocalGraphicsEnvironment();
	    GraphicsDevice gd = env.getDefaultScreenDevice();
	    return (WGLGraphicsConfig)gd.getDefaultConfiguration();
	}
    }

    public static class WGLWindowSurfaceData extends WGLSurfaceData {

	public WGLWindowSurfaceData(WComponentPeer peer, 
                                    WGLGraphicsConfig gc)
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

    public static class WGLOffScreenSurfaceData extends WGLSurfaceData {

	private Image offscreenImage;
	private int width, height;
	private int transparency;

	public WGLOffScreenSurfaceData(WComponentPeer peer,
                                       WGLGraphicsConfig gc, 
                                       int width, int height,
                                       Image image, ColorModel cm,
                                       int type)
        {
	    super(peer, gc,
                  (type == TEXTURE) ? OpenGLTexture :
                      (type == PBUFFER) ? OpenGLSurfaceRTT : OpenGLSurface,
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
