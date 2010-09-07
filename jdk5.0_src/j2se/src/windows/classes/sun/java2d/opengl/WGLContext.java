/*
 * @(#)WGLContext.java	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

public class WGLContext extends OGLContext {

    private native long initNativeContext(long pInfo);
    protected native boolean makeNativeContextCurrent(long pCtx,
                                                      long pSrc, long pDst);

    public WGLContext(WGLGraphicsConfig graphicsConfig) {
        long pInfo = graphicsConfig.getNativeConfigInfo();
        nativeContext = initNativeContext(pInfo);
    }
}
