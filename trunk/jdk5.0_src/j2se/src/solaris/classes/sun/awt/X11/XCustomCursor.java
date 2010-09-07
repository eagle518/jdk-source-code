/*
 * @(#)XCustomCursor.java	1.17 03/01/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.awt.X11CustomCursor;
import java.awt.*;
import java.awt.image.*;
import sun.awt.image.ImageRepresentation;

/**
 * A class to encapsulate a custom image-based cursor.  
 *
 * @see Component#setCursor
 * @version 	1.17 01/23/03
 * @author 	Thomas Ball
 * @author 	Bino George
 */
public class XCustomCursor extends X11CustomCursor {

    public XCustomCursor(Image cursor, Point hotSpot, String name) 
      throws IndexOutOfBoundsException {
        super(cursor, hotSpot, name);
    }

    /**
     * Returns the supported cursor size
     */
    static Dimension getBestCursorSize(int preferredWidth, int preferredHeight) {

        // Fix for bug 4212593 The Toolkit.createCustomCursor does not 
        //                     check absence of the image of cursor 
        // We use XQueryBestCursor which accepts unsigned ints to obtain
        // the largest cursor size that could be dislpayed 
        //Dimension d = new Dimension(Math.abs(preferredWidth), Math.abs(preferredHeight));
        Dimension d;
        try {
            XToolkit.awtLock();


            long display = XToolkit.getDisplay();
            long root_window = XlibWrapper.RootWindow(display,
                    XlibWrapper.DefaultScreen(display)); 

            XlibWrapper.XQueryBestCursor(display,root_window, Math.abs(preferredWidth),Math.abs(preferredHeight),XlibWrapper.larg1,XlibWrapper.larg2);
            d = new Dimension(XlibWrapper.unsafe.getInt(XlibWrapper.larg1),XlibWrapper.unsafe.getInt(XlibWrapper.larg2));
        }
        finally {
            XToolkit.awtUnlock();
        }
        return d;
    }
    
    protected void createCursor(byte[] xorMask, byte[] andMask, 
                                int width, int height, 
                                int fcolor, int bcolor, 
                                int xHotSpot, int yHotSpot)
    {
        try {
            XToolkit.awtLock();

            long display = XToolkit.getDisplay();
            long root_window = XlibWrapper.RootWindow(display,
                    XlibWrapper.DefaultScreen(display)); 

            long colormap = XToolkit.getDefaultXColormap();
            XColor fore_color = new XColor();

            fore_color.set_flags((byte) (XlibWrapper.DoRed | XlibWrapper.DoGreen | XlibWrapper.DoBlue));  
            fore_color.set_red((short)(((fcolor >> 16) & 0x000000ff) << 8));
            fore_color.set_green((short) (((fcolor >> 8) & 0x000000ff) << 8));
            fore_color.set_blue((short)(((fcolor >> 0) & 0x000000ff) << 8));

            XlibWrapper.XAllocColor(display,colormap,fore_color.pData);


            XColor back_color = new XColor();
            back_color.set_flags((byte) (XlibWrapper.DoRed | XlibWrapper.DoGreen | XlibWrapper.DoBlue));  

            back_color.set_red((short) (((bcolor >> 16) & 0x000000ff) << 8));
            back_color.set_green((short) (((bcolor >> 8) & 0x000000ff) << 8));
            back_color.set_blue((short) (((bcolor >> 0) & 0x000000ff) << 8));

            XlibWrapper.XAllocColor(display,colormap,back_color.pData);


            long nativeXorMask = Native.toData(xorMask); 
            long source = XlibWrapper.XCreateBitmapFromData(display,root_window,nativeXorMask,width,height);

            long nativeAndMask = Native.toData(andMask);
            long mask =  XlibWrapper.XCreateBitmapFromData(display,root_window,nativeAndMask,width,height);

            long cursor = XlibWrapper.XCreatePixmapCursor(display,source,mask,fore_color.pData,back_color.pData,xHotSpot,yHotSpot);

            XlibWrapper.unsafe.freeMemory(nativeXorMask); 
            XlibWrapper.unsafe.freeMemory(nativeAndMask); 
            XlibWrapper.XFreePixmap(display,source);
            XlibWrapper.XFreePixmap(display,mask);
            back_color.dispose();
            fore_color.dispose();

            XGlobalCursorManager.setPData(this,cursor); 
        }
        finally {
            XToolkit.awtUnlock();
        }
        
    }
}
