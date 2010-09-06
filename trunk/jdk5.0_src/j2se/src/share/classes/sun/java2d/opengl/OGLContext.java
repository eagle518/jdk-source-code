/*
 * @(#)OGLContext.java	1.2 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

import java.awt.AlphaComposite;
import java.awt.Composite;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Toolkit;
import java.awt.event.InvocationEvent;
import java.awt.geom.AffineTransform;
import sun.java2d.InvalidPipeException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.XORComposite;
import sun.java2d.pipe.Region;

public abstract class OGLContext {

    /**
     * Context flags (see OGLContext.h for more information on each flag).
     */
    public static final int NO_CONTEXT_FLAGS = 0;
    public static final int USE_EXTRA_ALPHA  = 1;
    public static final int SRC_IS_PREMULT   = 2;
    public static final int SRC_IS_OPAQUE    = 4;

    /**
     * The lock object used to synchronize access to the native windowing
     * system layer.  Note that rendering methods should always synchronize on
     * OGLContext.LOCK before calling the OGLContext.getContext() method,
     * or any other method that invokes native OpenGL commands.
     */
    public static Object LOCK;

    /**
     * This is a reference to the OGLContext whose native context is
     * current for the entire OpenGL pipeline (across all threads).  If
     * this value is null, it means that there is no current context.  It is
     * provided here so that validate() only needs to do a quick reference
     * check to see if the OGLContext passed to that method is the same as
     * the one we've cached here.
     */
    private static OGLContext currentContext;

    protected long         nativeContext;
    private OGLSurfaceData validatedSrcData;
    private OGLSurfaceData validatedDstData;
    private Region         validatedClip;
    private Composite      validatedComp;
    private int            validatedPixel;
    private int            validatedFlags;
    private boolean        xformInUse;

    private static native void initIDs();

    protected abstract boolean makeNativeContextCurrent(long pCtx,
                                                        long pSrc, long pDst);
    private native void setViewport(long pSrc, long pDst);
    private native void setClip(long pDst, Region clip, boolean isRect,
                                int x1, int y1, int x2, int y2);
    private native void resetComposite(long pCtx);
    private native void setAlphaComposite(long pCtx, int rule,
                                          float extraAlpha, int flags);
    private native void setXorComposite(long pCtx, int xorPixel);
    private native void setTransform(long pCtx,
                                     AffineTransform xform,
                                     double m00, double m10, double m01,
                                     double m11, double m02, double m12);
    private native void setColor(long pCtx, int pixel, int flags);

    static {
        if (!GraphicsEnvironment.isHeadless()) {
            initIDs();
        }
    }

    /**
     * Fetches the OGLContext associated with the current
     * thread/GraphicsConfig pair, validates the context using the given
     * parameters, then returns the handle to the native context object.
     * Most rendering operations will call this method first in order to
     * prepare the native OpenGL layer before issuing rendering commands.
     */
    public static long getContext(OGLSurfaceData srcData,
                                  OGLSurfaceData dstData,
                                  Region clip, Composite comp,
                                  AffineTransform xform,
                                  int pixel, int flags)
    {
        OGLContext oglc = dstData.getContext();
        oglc.validate(srcData, dstData, clip, comp, xform, pixel, flags);
        currentContext = oglc;
        return oglc.getNativeContext();
    }

    /**
     * Simplified version of getContext() that disables all native context
     * state settings before returning the context handle.
     */
    public static long getContext(OGLSurfaceData dstData) {
        return getContext(dstData, dstData,
                          null, null, null, 0, NO_CONTEXT_FLAGS);
    }

    /**
     * Makes the shared context current and returns a handle to the native
     * OGLContext object.  This method should be used for operations with
     * an OpenGL texture as the destination surface (e.g. a sw->texture
     * blit loop).
     */
    public static long getSharedContext(OGLSurfaceData dstData) {
        currentContext = null;
        return dstData.getSharedContext();
    }

    /**
     * Returns a handle to the native OGLContext structure associated with
     * this object.
     */
    public long getNativeContext() {
        return nativeContext;
    }

    /**
     * Makes this context "current" for the source (readable) surface and
     * destination (drawable) surface.  Returns true if the operation is
     * successful, and the currentContext is updated to refer to the newly
     * current context.  Otherwise, returns false and invalidates the
     * currentContext reference.
     */
    public boolean makeCurrent(OGLSurfaceData srcData,
                               OGLSurfaceData dstData)
    {
        int type = dstData.getType();

        if (type == OGLSurfaceData.UNDEFINED) {
            // the window surface has not yet been setup, so we will
            // initialize the native window before making the context current
            if (!dstData.initWindow()) {
                currentContext = null;
                return false;
            }
        }

        if (type == OGLSurfaceData.TEXTURE) {
            // cannot make a context current for a texture object (instead use
            // getSharedContext() to make the shared context current)
            return false;
        }

        long pSrc = srcData.getNativeOps();
        long pDst = dstData.getNativeOps();

        if (makeNativeContextCurrent(nativeContext, pSrc, pDst)) {
            currentContext = this;
            return true;
        }

        currentContext = null;
        return false;
    }

    /**
     * Validates the given parameters against the current state for this
     * context.  If this context is not current, it will be made current
     * for the given source and destination surfaces, and the viewport will
     * be updated.  Then each part of the context state (clip, composite,
     * etc.) is checked against the previous value.  If the value has changed
     * since the last call to validate(), it will be updated accordingly.
     */
    public void validate(OGLSurfaceData srcData, OGLSurfaceData dstData,
                         Region clip, Composite comp, AffineTransform xform,
                         int pixel, int flags)
    {
        long pDst = dstData.getNativeOps();
        boolean updateClip = false;

        if (!dstData.isValid()) {
            throw new InvalidPipeException("bounds changed");
        }

        if ((currentContext != this) ||
            (srcData != validatedSrcData) ||
            (dstData != validatedDstData))
        {
            long pSrc = srcData.getNativeOps();

            // invalidate pixel and clip (so they will be updated below)
            validatedPixel = ~pixel;
            updateClip = true;

            // attempt to make this context current
            if (!makeCurrent(srcData, dstData)) {
                // there was a problem making the context current, so return
                return;
            }

            validatedSrcData = srcData;
            validatedDstData = dstData;

            // update the viewport
            setViewport(pSrc, pDst);
        }

        // validate clip
        if ((clip != validatedClip) || updateClip) {
            validatedClip = clip;
            if (clip != null) {
                setClip(pDst, clip, clip.isRectangular(),
                        clip.getLoX(), clip.getLoY(),
                        clip.getHiX(), clip.getHiY());
            } else {
                setClip(pDst, null, false, 0, 0, 0, 0);
            }
        }

        // validate composite
        // REMIND: flag validation could be made less aggressive...
        if ((comp != validatedComp) || (flags != validatedFlags)) {
            // invalidate pixel
            validatedPixel = ~pixel;
            validatedComp = comp;
            if (comp != null) {
                if (comp instanceof XORComposite) {
                    int xorPixel = ((XORComposite)comp).getXorPixel();
                    setXorComposite(nativeContext, xorPixel);
                } else {
                    AlphaComposite ac = (AlphaComposite)comp;
                    setAlphaComposite(nativeContext, ac.getRule(),
                                      ac.getAlpha(), flags);
                }
            } else {
                resetComposite(nativeContext);
            }
        }

        // validate transform
        if (xform == null) {
            if (xformInUse) {
                setTransform(nativeContext, null, 0, 0, 0, 0, 0, 0);
                xformInUse = false;
            }
        } else {
            double[] mtx = new double[6];
            xform.getMatrix(mtx);
            setTransform(nativeContext, xform,
                         mtx[0], mtx[1], mtx[2], mtx[3], mtx[4], mtx[5]);
            xformInUse = true;
        }

        // validate pixel
        if (pixel != validatedPixel) {
            validatedPixel = pixel;
            setColor(nativeContext, pixel, flags);
        }

        // save flags for later comparison
        validatedFlags = flags;

        // mark dstData dirty
        dstData.markDirty();
    }

    /**
     * Invalidates the currentContext field to ensure that we properly
     * revalidate the OGLContext (make it current, etc.) next time through
     * the validate() method.  This is typically invoked from native methods
     * that affect the current context state (e.g. disposing a context or
     * surface).
     */
    public static void invalidateCurrentContext() {
        currentContext = null;
    }

    /**
     * The following methods are responsible for flushing the OpenGL
     * pipeline in an asynchronous manner.  This ensures that flush events
     * occur on the appropriate thread (the EventDispatchThread in this
     * case), and only at periodic intervals (to avoid calling the native
     * glFlush() method too often, which results in poor performance).
     */

    private static native void flushPipeline();
    private static InvocationEvent flushEvt;
    private static boolean flushPending = false;

    private static class NativeGLFlush implements Runnable {
        public void run() {
            synchronized (LOCK) {
                // REMIND: should ensure that we have a current context...
                flushPipeline();
                flushPending = false;
            }
        }
    }

    private static void invokeNativeGLFlush() {
        if (!flushPending) {
            flushPending = true;
            if (flushEvt == null) {
                // ideally this would be initialized in a static block, but
                // the getDefaultToolkit() call could cause re-entrance into
                // that method, which leads to initializing the Toolkit twice
                // (a bad thing)
                flushEvt = new InvocationEvent(Toolkit.getDefaultToolkit(),
                                               new NativeGLFlush());
            }
            Toolkit.getDefaultToolkit().
                getSystemEventQueue().postEvent(flushEvt);
        }
    }
}
