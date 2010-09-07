/*
 * @(#)GLXGraphicsConfig.java	1.11 04/04/02
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
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DirectColorModel;
import java.awt.image.ImageProducer;
import java.awt.image.VolatileImage;
import java.awt.image.WritableRaster;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11GraphicsDevice;
import sun.awt.image.SunVolatileImage;
import sun.awt.X11ComponentPeer;
import sun.java2d.SurfaceData;
import sun.java2d.loops.SurfaceType;

public class GLXGraphicsConfig extends X11GraphicsConfig {

    protected static boolean glxAvailable;
    protected static boolean glxVerbose;
    protected boolean blendPremultAvailable;
    protected boolean texNonPow2Available;

    private static native boolean initGLX();
    private static native long getGLXConfigInfo(int screennum, int visualnum);
    private static native boolean isDoubleBuffered(long configInfo);
    private static native boolean isBlendPremultAvailable(long configInfo);
    private static native boolean isTexNonPow2Available(long configInfo);
    private native void initConfig(long ctxinfo);

    static {
        boolean tryglx = false;
        String oglProp = (String)java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.java2d.opengl"));
	if (oglProp != null) {
	    if (oglProp.equals("true") || oglProp.equals("t")) {
		tryglx = true;
            } else if (oglProp.equals("True") || oglProp.equals("T")) {
                tryglx = true;
                glxVerbose = true;
            }
        }
        if (tryglx) {
            OGLContext.LOCK = sun.awt.X11GraphicsEnvironment.class;
            synchronized (OGLContext.LOCK) {
                glxAvailable = initGLX();
            }
            if (glxVerbose && !glxAvailable) {
                System.out.println("Could not enable OpenGL pipeline " +
                                   "(GLX 1.3 not available)");
            }
        }
    }

    protected GLXGraphicsConfig(X11GraphicsDevice device, int visualnum,
                                boolean doubleBuffer, long configInfo)
    {
        super(device, visualnum, 0, 0, doubleBuffer);
        initConfig(configInfo);
        blendPremultAvailable = isBlendPremultAvailable(configInfo);
        texNonPow2Available = isTexNonPow2Available(configInfo);
    }

    public static GLXGraphicsConfig getConfig(X11GraphicsDevice device,
                                              int visualnum)
    {
        if (!glxAvailable) {
            return null;
        }

        long cfginfo = 0;
        synchronized (OGLContext.LOCK) {
            cfginfo = getGLXConfigInfo(device.getScreen(), visualnum);
        }
        if (cfginfo == 0) {
            return null;
        }

        boolean dblbuffer = isDoubleBuffered(cfginfo);

        return new GLXGraphicsConfig(device, visualnum, dblbuffer, cfginfo);
    }

    public static boolean isGLXAvailable() {
        return glxAvailable;
    }

    public static boolean isGLXVerbose() {
        return glxVerbose;
    }

    public boolean isBlendPremultAvailable() {
        return blendPremultAvailable;
    }

    public boolean isTexNonPow2Available() {
        return texNonPow2Available;
    }

    private ThreadLocal<OGLContext> contextTls =
        new ThreadLocal<OGLContext>() {
            protected OGLContext initialValue() {
                return new GLXContext(GLXGraphicsConfig.this);
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

    public BufferedImage createCompatibleImage(int width, int height) {
        ColorModel model = new DirectColorModel(24, 0xff0000, 0xff00, 0xff);
	WritableRaster
            raster = model.createCompatibleWritableRaster(width, height);
	return new BufferedImage(model, raster, model.isAlphaPremultiplied(),
				 null);
    }

    public VolatileImage createCompatibleVolatileImage(int width, int height) {
        return new SunVolatileImage(this, width, height);
    }

    public BufferedImage createCompatibleImage(int width, int height,
                                               int transparency)
    {
        switch (transparency) {
        case Transparency.OPAQUE:
            return createCompatibleImage(width, height);
            
        case Transparency.BITMASK:
        case Transparency.TRANSLUCENT:
            ColorModel cm = getColorModel(transparency);
            WritableRaster wr = cm.createCompatibleWritableRaster(width,
                                                                  height);
            return new BufferedImage(cm, wr,
                                     cm.isAlphaPremultiplied(), null);

        default:
            throw new IllegalArgumentException("Unknown transparency type "+
                                               transparency);
        }
    }

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

    public String toString() {
	return ("GLXGraphicsConfig[dev="+screen+
		",vis=0x"+Integer.toHexString(visual)+
		"]");
    }

    /**
     * The following methods are invoked from MToolkit or XToolkit.java and
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
        return GLXSurfaceData.createData(peer);
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
        return new GLXRemoteOffScreenImage(target, model, wr,
                                           model.isAlphaPremultiplied());
    }

    /**
     * The following methods correspond to the multibuffering methods in
     * X11ComponentPeer.java...
     */

    private native void swapBuffers(long window);

    /**
     * Attempts to create a GLX-based backbuffer for the given peer.  If
     * the requested configuration is not natively supported, an AWTException
     * is thrown.  Otherwise, if the backbuffer creation is successful, a
     * value of 1 is returned.
     */
    public long createBackBuffer(X11ComponentPeer peer,
                                 int numBuffers, BufferCapabilities caps)
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

        // non-zero return value means backbuffer creation was successful
        // (checked in X11ComponentPeer.flip(), etc.)
        return 1;
    }

    /**
     * Destroys the backbuffer object represented by the given handle value.
     */
    public void destroyBackBuffer(long backBuffer) {
        // REMIND: mark native backbuffer as unused
    }

    /**
     * Creates a VolatileImage that essentially wraps the target Component's
     * backbuffer (the provided backbuffer handle is essentially ignored).
     */
    public VolatileImage createBackBufferImage(Component target,
                                               long backBuffer)
    {
        return new SunVolatileImage(target,
                                    target.getWidth(), target.getHeight(),
                                    Boolean.TRUE);
    }

    /**
     * Performs the native GLX flip operation for the given target Component.
     */
    public void flip(X11ComponentPeer peer,
                     Component target, VolatileImage xBackBuffer,
                     BufferCapabilities.FlipContents flipAction)
    {
        long window = peer.getContentWindow();

        // REMIND: when the peer is resized, the bounds of the
        //         backbuffer image are no longer valid, which will be
        //         problematic for COPIED/BACKGROUND...
        if (flipAction == BufferCapabilities.FlipContents.COPIED) {
            Graphics g = peer.getGraphics();
            try {
                g.drawImage(xBackBuffer, 0, 0, null);
            } finally {
                g.dispose();
            }
            return;
        } else if (flipAction == BufferCapabilities.FlipContents.PRIOR) {
            // not supported by GLX...
            return;
        }

        synchronized (OGLContext.LOCK) {
            swapBuffers(window);
        }

        if (flipAction == BufferCapabilities.FlipContents.BACKGROUND) {
            Graphics g = xBackBuffer.getGraphics();
            try {
                g.setColor(target.getBackground());
                g.fillRect(0, 0,
                           xBackBuffer.getWidth(),
                           xBackBuffer.getHeight());
            } finally {
                g.dispose();
            }
        }
    }
}
