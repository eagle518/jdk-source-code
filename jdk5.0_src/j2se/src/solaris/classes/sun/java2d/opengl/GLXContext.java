/*
 * @(#)GLXContext.java	1.1 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.opengl;

public class GLXContext extends OGLContext {

    private native long initNativeContext(long aData);
    protected native boolean makeNativeContextCurrent(long pCtx,
                                                      long pSrc, long pDst);

    public GLXContext(GLXGraphicsConfig graphicsConfig) {
        long aData = graphicsConfig.getAData();
        nativeContext = initNativeContext(aData);
    }
}
