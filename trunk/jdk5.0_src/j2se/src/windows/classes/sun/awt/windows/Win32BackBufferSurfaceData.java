/*
 * @(#)Win32BackBufferSurfaceData.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.Win32GraphicsDevice;
import sun.java2d.loops.SurfaceType;

public class Win32BackBufferSurfaceData extends Win32OffScreenSurfaceData {

    private Win32SurfaceData parentData;

    /**
     * Private constructor.  Use createData() to create an object.
     */
    private Win32BackBufferSurfaceData(int width, int height,
                                       SurfaceType sType, ColorModel cm,
                                       GraphicsConfiguration gc,
                                       Image image, int screen,
                                       Win32SurfaceData parentData)
    {
        super(width, height, sType, cm, gc, image, Transparency.OPAQUE);
        this.parentData = parentData;
        initSurface(cm.getPixelSize(), width, height, screen, parentData);
        initD3DPipes();
    }

    private native void initSurface(int depth, int width, int height,
                                    int screen, Win32SurfaceData parentData);

    public void restoreSurface() {
        parentData.restoreSurface();
    }

    public static Win32BackBufferSurfaceData
        createData(int width, int height,
                   ColorModel cm, GraphicsConfiguration gc, Image image,
                   Win32SurfaceData parentData)
    {
        Win32GraphicsDevice gd = (Win32GraphicsDevice)gc.getDevice();
        SurfaceType sType = getSurfaceType(cm, Transparency.OPAQUE);
        return new Win32BackBufferSurfaceData(width, height, sType,
                                              cm, gc, image,
                                              gd.getScreen(), parentData);
    }
}
