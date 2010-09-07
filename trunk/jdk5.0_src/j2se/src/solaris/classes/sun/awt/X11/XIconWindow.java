/*
 * @(#)XIconWindow.java	1.1 03/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.image.*;
import sun.awt.X11GraphicsConfig;

import java.util.logging.*;

public class XIconWindow extends XBaseWindow {
    private final static Logger log = Logger.getLogger("sun.awt.X11.XIconWindow");
    XFramePeer parent;
    Dimension size;
    long iconPixmap;
    int iconWidth, iconHeight;    
    XIconWindow(XFramePeer parent) {
        super(new XCreateWindowParams(new Object[] {
            PARENT, parent,
            DELAYED, Boolean.TRUE}));
    }

    void instantPreInit(XCreateWindowParams params) {
        super.instantPreInit(params);
        this.parent = (XFramePeer)params.get(PARENT);
    }

    /**
     * @return array of XIconsSize structures, caller must free this array after use.
     */
    private XIconSize[] getIconSizes() {
        XToolkit.awtLock();
        try {
            AwtGraphicsConfigData adata = parent.getGraphicsConfigurationData();
            final long screen = adata.get_awt_visInfo().get_screen();
            final long display = XToolkit.getDisplay();

            if (log.isLoggable(Level.FINEST)) log.finest(adata.toString());

            long status =
                XlibWrapper.XGetIconSizes(display, XlibWrapper.RootWindow(display, screen), 
                                          XlibWrapper.larg1, XlibWrapper.iarg1);
            if (status == 0) {
                return null;
            }
            int count = Native.getInt(XlibWrapper.iarg1);
            long sizes_ptr = Native.getLong(XlibWrapper.larg1); // XIconSize*
            log.log(Level.FINEST, "count = {1}, sizes_ptr = {0}", new Object[] {new Long(sizes_ptr), new Integer(count)});
            XIconSize[] res = new XIconSize[count];
            for (int i = 0; i < count; i++, sizes_ptr += XIconSize.getSize()) {
                res[i] = new XIconSize(sizes_ptr);
                log.log(Level.FINEST, "sizes_ptr[{1}] = {0}", new Object[] {res[i], new Integer(i)});
            }
            return res;
        } finally {
            XToolkit.awtUnlock();
        }
    }

    private Dimension calcIconSize(int widthHint, int heightHint) {
        XIconSize[] sizeList = getIconSizes();
        if (sizeList == null) {
            // No icon sizes so we simply fall back to 16x16
            return new Dimension(16, 16);
        }
        boolean found = false;
        int dist = 0xffffffff, newDist, diff = 0, closestHeight, closestWidth;
        int saveWidth = 0, saveHeight = 0;
        for (int i = 0; i < sizeList.length; i++) {
            if (widthHint >= sizeList[i].get_min_width() &&
                widthHint <= sizeList[i].get_max_width() &&
                heightHint >= sizeList[i].get_min_height() &&
                heightHint <= sizeList[i].get_max_height()) {
                found = true;
                if ((((widthHint-sizeList[i].get_min_width())
                      % sizeList[i].get_width_inc()) == 0) &&
                    (((heightHint-sizeList[i].get_min_height())
                      % sizeList[i].get_height_inc()) ==0)) {
                    /* Found an exact match */
                    saveWidth = widthHint;
                    saveHeight = heightHint;
                    dist = 0;
                    break;
                }
                diff = widthHint - sizeList[i].get_min_width();
                if (diff == 0) {
                    closestWidth = widthHint;
                } else {
                    diff = diff%sizeList[i].get_width_inc();
                    closestWidth = widthHint - diff;
                }
                diff = heightHint - sizeList[i].get_min_height();
                if (diff == 0) {
                    closestHeight = heightHint;
                } else {
                    diff = diff%sizeList[i].get_height_inc();
                    closestHeight = heightHint - diff;
                }
                newDist = closestWidth*closestWidth +
                    closestHeight*closestHeight;
                if (dist > newDist) {
                    saveWidth = closestWidth;
                    saveHeight = closestHeight;
                    dist = newDist;
                }
            }
        }
        if (log.isLoggable(Level.FINEST)) {
            log.finest("found=" + found);
        }
        if (!found) {
            if (log.isLoggable(Level.FINEST)) {
                log.finest("widthHint=" + widthHint + ", heightHint=" + heightHint
                           + ", saveWidth=" + saveWidth + ", saveHeight=" + saveHeight
                           + ", max_width=" + sizeList[0].get_max_width()
                           + ", max_height=" + sizeList[0].get_max_height()
                           + ", min_width=" + sizeList[0].get_min_width()
                           + ", min_height=" + sizeList[0].get_min_height());
            }

            if (widthHint  > sizeList[0].get_max_width() ||
                heightHint > sizeList[0].get_max_height())
            {
                // Icon image too big
                /* determine which way to scale */
                int wdiff = widthHint - sizeList[0].get_max_width();
                int hdiff = heightHint - sizeList[0].get_max_height();
                if (log.isLoggable(Level.FINEST)) {
                    log.finest("wdiff=" + wdiff + ", hdiff=" + hdiff);
                }
                if (wdiff >= hdiff) { /* need to scale width more  */
                    saveWidth = sizeList[0].get_max_width();
                    saveHeight =
                        (int)(((double)sizeList[0].get_max_width()/widthHint) * heightHint);
                } else {
                    saveWidth =
                        (int)(((double)sizeList[0].get_max_height()/heightHint) * widthHint);
                    saveHeight = sizeList[0].get_max_height();
                }
            } else if (widthHint  < sizeList[0].get_min_width() ||
                       heightHint < sizeList[0].get_min_height())
            {
                // Icon image too small
                saveWidth = (sizeList[0].get_min_width()+sizeList[0].get_max_width())/2;
                saveHeight = (sizeList[0].get_min_height()+sizeList[0].get_max_height())/2;
            } else {
                // Icon image fits within right size
                saveWidth = widthHint;
                saveHeight = widthHint;
            }
        }

        synchronized (getAWTLock()) {
            XlibWrapper.XFree(sizeList[0].pData);
        }
        if (log.isLoggable(Level.FINEST)) {
            log.finest("return " + saveWidth + "x" + saveHeight);
        }
        return new Dimension(saveWidth, saveHeight);
    }

    Dimension getIconSize(int widthHint, int heightHint) {
        if (size == null) {
            size = calcIconSize(widthHint, heightHint);
        }
        return size;
    }
   
    void setIconImage(int iconWidth, int iconHeight, DataBuffer data) {
        if (log.isLoggable(Level.FINEST)) {
            log.finest("iconsWidth = " + iconWidth + ", iconHeight = " + iconHeight);
        }
        XToolkit.awtLock();
        try {
            AwtGraphicsConfigData adata = parent.getGraphicsConfigurationData();
            awtImageData awtImage = adata.get_awtImage(0);
            XVisualInfo visInfo = adata.get_awt_visInfo();
            if (iconPixmap != 0
                && (this.iconWidth != iconWidth || this.iconHeight != iconHeight))
            {
                XlibWrapper.XFreePixmap(XToolkit.getDisplay(), iconPixmap);
                iconPixmap = 0;
                log.finest("Freed previous pixmap");
            }

            if (iconPixmap == 0) {
                iconPixmap = XlibWrapper.XCreatePixmap(XToolkit.getDisplay(), 
                                                       XlibWrapper.RootWindow(XToolkit.getDisplay(), visInfo.get_screen()),
                                                       iconWidth,
                                                       iconHeight,
                                                       awtImage.get_Depth() // adata->awtImage->Depth
                                                       );
                if (iconPixmap == 0) {
                    log.finest("Can't create new pixmap for icon");
                    return;
                } else {
                    log.finest("New pixmap for icon created");
                }
                this.iconWidth = iconWidth;
                this.iconHeight = iconHeight;
            }
            long bytes = 0;
            try {
                if (data instanceof DataBufferByte) {
                    byte[] buf = ((DataBufferByte)data).getData();
                    ColorData cdata = adata.get_color_data(0);
                    int num_colors = cdata.get_awt_numICMcolors();
                    for (int i = 0; i < buf.length; i++) {
                        buf[i] = (buf[i] >= num_colors) ?
                            0 : cdata.get_awt_icmLUT2Colors(buf[i]);
//                              0 : Native.getByte(cdata.get_awt_icmLUT2Colors(), buf[i]); // !!!! ERROR - array has unsigned index, byte is signed
                    }
                    bytes = Native.toData(buf);
                } else if (data instanceof DataBufferInt) {
                    bytes = Native.toData(((DataBufferInt)data).getData());
                } else if (data instanceof DataBufferUShort) {
                    bytes = Native.toData(((DataBufferUShort)data).getData());
                } else {
                    throw new IllegalArgumentException("Unknown data buffer: " + data);
                }
                int bpp = awtImage.get_wsImageFormat().get_bits_per_pixel();
                int slp =awtImage.get_wsImageFormat().get_scanline_pad(); 
                int bpsl = XlibWrapper.paddedwidth(iconWidth*bpp, slp) >> 3;
                if (((bpsl << 3) / bpp) < iconWidth) {
                    log.finest("Image format doesn't fit to icon width");
                    return;
                }
                long dst = XlibWrapper.XCreateImage(XToolkit.getDisplay(),
                                                    visInfo.get_visual(),
                                                    (int)awtImage.get_Depth(),                                               
                                                    (int)XlibWrapper.ZPixmap,
                                                    0,
                                                    bytes,
                                                    iconWidth,
                                                    iconHeight,
                                                    32,
                                                    bpsl);
                if (dst == 0) {
                    log.finest("Can't create XImage for icon");
                    return;
                } else {
                    log.finest("Created XImage for icon");
                }
                try {
                    long gc = XlibWrapper.XCreateGC(XToolkit.getDisplay(), iconPixmap, 0, 0);
                    if (gc == 0) {
                        log.finest("Can't create GC for pixmap");
                        return;
                    } else {
                        log.finest("Created GC for pixmap");
                    }
                    try {
                        XlibWrapper.XPutImage(XToolkit.getDisplay(), iconPixmap, gc,
                                              dst, 0, 0, 0, 0, iconWidth, iconHeight);
                    } finally {
                        XlibWrapper.XFreeGC(XToolkit.getDisplay(), gc);
                    }
                } finally {
                    XlibWrapper.XDestroyImage(dst);
                }                
            } finally {
                XlibWrapper.unsafe.freeMemory(bytes);
            }
            XWMHints hints = parent.getWMHints();
            window = hints.get_icon_window();
            if (window == 0) {
                log.finest("Icon window wasn't set");
                XCreateWindowParams params = getDelayedParams();
                params.add(BORDER_PIXEL, new Long(XToolkit.getAwtDefaultFg()));
                params.add(BACKGROUND_PIXMAP, iconPixmap);
                params.add(COLORMAP, adata.get_awt_cmap());
                params.add(DEPTH, awtImage.get_Depth());
                params.add(VISUAL_CLASS, (int)XlibWrapper.InputOutput);
                params.add(VISUAL, visInfo.get_visual());
                params.add(VALUE_MASK, XlibWrapper.CWBorderPixel | XlibWrapper.CWColormap | XlibWrapper.CWBackPixmap);
                params.add(PARENT_WINDOW, XlibWrapper.RootWindow(XToolkit.getDisplay(), visInfo.get_screen()));
                params.add(BOUNDS, new Rectangle(0, 0, iconWidth, iconHeight));
                params.remove(DELAYED);
                init(params);
                if (getWindow() == 0) {
                    log.finest("Can't create new icon window");
                    // Can't create window - provide pixmap
                    hints.set_flags(hints.get_flags() | XlibWrapper.IconPixmapHint);
                    hints.set_icon_pixmap(iconPixmap);
                    XlibWrapper.XSetWMHints(XToolkit.getDisplay(), parent.getShell(), hints.pData);
                    return;
                } else {
                    log.finest("Created new icon window");
                }
            }
            // Provide both pixmap and window, WM or Taskbar will use the one they find more appropriate
            hints.set_flags(hints.get_flags() | XlibWrapper.IconWindowHint | XlibWrapper.IconPixmapHint);
            hints.set_icon_pixmap(iconPixmap);
            hints.set_icon_window(window);
            XlibWrapper.XSetWMHints(XToolkit.getDisplay(), parent.getShell(), hints.pData);                
            log.finest("Set icon window hint");
            XlibWrapper.XSetWindowBackgroundPixmap(XToolkit.getDisplay(), getWindow(), iconPixmap);
            XlibWrapper.XClearWindow(XToolkit.getDisplay(), getWindow());
        } finally {
            XToolkit.awtUnlock();
        }
    }
}
