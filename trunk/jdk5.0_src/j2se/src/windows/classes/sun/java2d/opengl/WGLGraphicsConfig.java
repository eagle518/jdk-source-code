/*
 * @(#)WGLGraphicsConfig.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AWTException;
import java.awt.BufferCapabilities;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.ImageCapabilities;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DirectColorModel;
import java.awt.image.ImageProducer;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import sun.awt.Win32GraphicsConfig;
import sun.awt.Win32GraphicsDevice;
import sun.awt.image.SunVolatileImage;
import sun.awt.windows.WComponentPeer;
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;

public class WGLGraphicsConfig extends Win32GraphicsConfig {

    protected static boolean wglAvailable;
    private static ImageCapabilities imageCaps = new ImageCapabilities(true);

    protected boolean doubleBuffer;
    protected boolean blendPremultAvailable;
    protected boolean texNonPow2Available;
    private BufferCapabilities bufferCaps;
    private long pConfigInfo;

    public static native int getDefaultPixFmt(int screennum);
    private static native boolean initWGL();
    private static native long getWGLConfigInfo(int screennum, int visualnum);
    private static native boolean isDoubleBuffered(long configInfo);
    private static native boolean isBlendPremultAvailable(long configInfo);
    private static native boolean isTexNonPow2Available(long configInfo);

    static {
        OGLContext.LOCK = sun.awt.Win32GraphicsEnvironment.class;
        synchronized (OGLContext.LOCK) {
            wglAvailable = initWGL();
        }
    }

    protected WGLGraphicsConfig(Win32GraphicsDevice device, int visualnum,
                                boolean doubleBuffer, long configInfo)
    {
        super(device, visualnum);
        this.pConfigInfo = configInfo;
        this.doubleBuffer = doubleBuffer;
        blendPremultAvailable = isBlendPremultAvailable(configInfo);
        texNonPow2Available = isTexNonPow2Available(configInfo);
    }

    public static WGLGraphicsConfig getConfig(Win32GraphicsDevice device,
                                              int pixfmt)
    {
        if (!wglAvailable) {
            return null;
        }

        long cfginfo = 0;
        synchronized (OGLContext.LOCK) {
            cfginfo = getWGLConfigInfo(device.getScreen(), pixfmt);
        }
        if (cfginfo == 0) {
            return null;
        }

        boolean dblbuffer = isDoubleBuffered(cfginfo);

        return new WGLGraphicsConfig(device, pixfmt, dblbuffer, cfginfo);
    }

    public static boolean isWGLAvailable() {
        return wglAvailable;
    }

    public boolean isBlendPremultAvailable() {
        return blendPremultAvailable;
    }

    public boolean isTexNonPow2Available() {
        return texNonPow2Available;
    }

    public boolean isDoubleBuffered() {
        return doubleBuffer;
    }

    public long getNativeConfigInfo() {
        return pConfigInfo;
    }

    private ThreadLocal<OGLContext> contextTls =
        new ThreadLocal<OGLContext>() {
            protected OGLContext initialValue() {
                return new WGLContext(WGLGraphicsConfig.this);
            }
        };

    public final OGLContext getContext() {
        return contextTls.get();
    }

    private static native long initNativeSharedContext();
    private static native long makeNativeSharedContextCurrent(long pCtx);

    private static ThreadLocal<Long> sharedContextTls =
        new ThreadLocal<Long>() {
            protected Long initialValue() {
                return new Long(initNativeSharedContext());
            }
        };

    public static long getThreadSharedContext() {
        long pCtx = sharedContextTls.get().longValue();
        return makeNativeSharedContextCurrent(pCtx);
    }

    @Override
    public ColorModel getColorModel(int transparency) {
	switch (transparency) {
	case Transparency.OPAQUE:
            // REMIND: once the ColorModel spec is changed, this should be
            //         an opaque premultiplied DCM...
            return new DirectColorModel(24, 0xff0000, 0xff00, 0xff);
	case Transparency.BITMASK:
            return new DirectColorModel(25, 0xff0000, 0xff00, 0xff, 0x1000000);
	case Transparency.TRANSLUCENT:
            ColorSpace cs = ColorSpace.getInstance(ColorSpace.CS_sRGB);
            return new DirectColorModel(cs, 32,
                                        0xff0000, 0xff00, 0xff, 0xff000000,
                                        true, DataBuffer.TYPE_INT);
	default:
	    return null;
        }
    }

    @Override
    public String toString() {
	return ("WGLGraphicsConfig[dev="+screen+",pixfmt="+visual+"]");
    }

    /**
     * The following methods are invoked from WComponentPeer.java rather
     * than having the Win32-dependent implementations hardcoded in that
     * class.  This way the appropriate actions are taken based on the peer's
     * GraphicsConfig, whether it is a Win32GraphicsConfig or a
     * WGLGraphicsConfig.
     */

    /**
     * Creates a new SurfaceData that will be associated with the given
     * WComponentPeer.
     */
    @Override
    public SurfaceData createSurfaceData(WComponentPeer peer,
                                         int numBackBuffers)
    {
        return WGLSurfaceData.createData(peer);
    }

    /**
     * Creates a new hidden-acceleration image of the given width and height
     * that is associated with the target Component.
     */
    @Override
    public Image createAcceleratedImage(Component target,
                                        int width, int height)
    {
        ColorModel model = getColorModel(Transparency.OPAQUE);
        WritableRaster wr = 
            model.createCompatibleWritableRaster(width, height);
        return new WGLOffScreenImage(target, model, wr,
                                     model.isAlphaPremultiplied());
    }

    /**
     * The following methods correspond to the multibuffering methods in
     * WComponentPeer.java...
     */

    private native void swapBuffers(long pPeerData);

    /**
     * Checks that the requested configuration is natively supported; if not,
     * an AWTException is thrown.
     */
    @Override
    public void assertOperationSupported(Component target,
                                         int numBuffers,
                                         BufferCapabilities caps)
        throws AWTException
    {
        if (numBuffers > 2) {
            throw new AWTException(
                "Only double or single buffering is supported");
        }
        BufferCapabilities configCaps = getBufferCapabilities();
        if (!configCaps.isPageFlipping()) {
            throw new AWTException("Page flipping is not supported");
        }
        if (caps.getFlipContents() == BufferCapabilities.FlipContents.PRIOR) {
            throw new AWTException("FlipContents.PRIOR is not supported");
        }
    }

    /**
     * Creates a WGL-based backbuffer for the given peer and returns the
     * image wrapper.
     */
    @Override
    public VolatileImage createBackBuffer(WComponentPeer peer) {
        Component target = (Component)peer.getTarget();
        return new SunVolatileImage(target,
                                    target.getWidth(), target.getHeight(),
                                    Boolean.TRUE);
    }

    /**
     * Performs the native WGL flip operation for the given target Component.
     */
    @Override
    public void flip(WComponentPeer peer,
                     Component target, VolatileImage backBuffer,
                     BufferCapabilities.FlipContents flipAction)
    {
        long pPeerData = peer.getData();

        // REMIND: when the peer is resized, the bounds of the
        //         backbuffer image are no longer valid, which will be
        //         problematic for COPIED/BACKGROUND...
        if (flipAction == BufferCapabilities.FlipContents.COPIED) {
            Graphics g = peer.getGraphics();
            try {
                g.drawImage(backBuffer, 0, 0, null);
            } finally {
                g.dispose();
            }
            return;
        } else if (flipAction == BufferCapabilities.FlipContents.PRIOR) {
            // not supported by WGL...
            return;
        }

        synchronized (OGLContext.LOCK) {
            swapBuffers(pPeerData);
        }

        if (flipAction == BufferCapabilities.FlipContents.BACKGROUND) {
            Graphics g = backBuffer.getGraphics();
            try {
                g.setColor(target.getBackground());
                g.fillRect(0, 0,
                           backBuffer.getWidth(),
                           backBuffer.getHeight());
            } finally {
                g.dispose();
            }
        }
    }

    private static class WGLCapabilities extends BufferCapabilities {
        public WGLCapabilities(boolean dblBuf) {
            super(imageCaps, imageCaps,
                  dblBuf ? FlipContents.UNDEFINED : null);
        }
    }
    
    @Override
    public BufferCapabilities getBufferCapabilities() {
        if (bufferCaps == null) {
            bufferCaps = new WGLCapabilities(doubleBuffer);
        }
        return bufferCaps;
    }

    @Override
    public ImageCapabilities getImageCapabilities() {
        return imageCaps;
    }
}
