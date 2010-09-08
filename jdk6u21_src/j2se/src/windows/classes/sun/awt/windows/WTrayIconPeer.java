/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved. Use is
 * subject to license terms.
 */

package sun.awt.windows;

import java.awt.Image;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.Graphics;
import java.awt.AWTEvent;
import java.awt.Frame;
import java.awt.PopupMenu;
import java.awt.Point;
import java.awt.TrayIcon;
import java.awt.Image;
import java.awt.peer.TrayIconPeer;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.event.InvocationEvent;
import java.awt.Toolkit;
import java.awt.image.*;
import sun.awt.image.IntegerComponentRaster;
import sun.awt.windows.WPopupMenuPeer;
import java.lang.reflect.Field;

public class WTrayIconPeer extends WObjectPeer implements TrayIconPeer {
    final static int TRAY_ICON_WIDTH = 16;
    final static int TRAY_ICON_HEIGHT = 16;
    final static int TRAY_ICON_MASK_SIZE = (TRAY_ICON_WIDTH * TRAY_ICON_HEIGHT) / 8;

    IconObserver observer = new IconObserver();
    boolean firstUpdate = true;
    Frame popupParent = new Frame();
    PopupMenu popup;

    protected void disposeImpl() {
        if (popupParent != null) {
            popupParent.dispose();
        }
        popupParent.dispose();
        _dispose();
        WToolkit.targetDisposedPeer(target, this);
    }

    WTrayIconPeer(TrayIcon target) {
        this.target = target;
        popupParent.addNotify();
        create();
        updateImage();
    }

    public void updateImage() {
        Image image = ((TrayIcon)target).getImage();
        if (image != null) {
            updateNativeImage(image);
        }        
    }

    public native void setToolTip(String tooltip);

    // This is currently called from TrayIcon.processMouseEvent,
    // but it's better to call it from native event handler,
    // however in this case there will be problems with keyboard menu navigation.
    public synchronized void showPopupMenu(final int x, final int y) {
        if (isDisposed())
            return;

        EventQueue.invokeLater(new Runnable() {
                public void run() {
                    PopupMenu newPopup = ((TrayIcon)target).getPopupMenu();
                    if (popup != newPopup) {
                        if (popup != null) {
                            popupParent.remove(popup);
                        }
                        if (newPopup != null) {
                            popupParent.add(newPopup);
                        }
                        popup = newPopup;
                    }
                    if (popup != null) {
                        ((WPopupMenuPeer)popup.getPeer()).show(popupParent, new Point(x, y));
                    }
                }
            });
    }

    public void displayMessage(String caption, String text, String messageType) {
        // The situation when both caption and text are null is processed in the shared code.
        if (caption == null) {
            caption = "";
        }
        if (text == null) {
            text = "";
        }
        _displayMessage(caption, text, messageType);
    }


    // ***********************************************
    // ***********************************************


    synchronized void updateNativeImage(Image image) {
        if (isDisposed())
            return;

        boolean autosize = ((TrayIcon)target).isImageAutoSize();

        BufferedImage bufImage = new BufferedImage(TRAY_ICON_WIDTH, TRAY_ICON_HEIGHT,
                                                   BufferedImage.TYPE_INT_ARGB);
        Graphics2D gr = bufImage.createGraphics();
        if (gr != null) {
            try {
                gr.setPaintMode();

                gr.drawImage(image, 0, 0, (autosize ? TRAY_ICON_WIDTH : image.getWidth(observer)),
                             (autosize ? TRAY_ICON_HEIGHT : image.getHeight(observer)), observer);

                createNativeImage(bufImage);

                updateNativeIcon(!firstUpdate);
                if (firstUpdate) firstUpdate = false;

            } finally {
                gr.dispose();                
            }
        }
    }

    void createNativeImage(BufferedImage bimage) {
        Raster raster = bimage.getRaster();
        byte[] andMask = new byte[TRAY_ICON_MASK_SIZE];
        int  pixels[] = ((DataBufferInt)raster.getDataBuffer()).getData();
        int npixels = pixels.length;
        int ficW = raster.getWidth();

        for (int i = 0; i < npixels; i++) {
            int ibyte = i / 8;
            int omask = 1 << (7 - (i % 8));

            if ((pixels[i] & 0xff000000) == 0) {
                // Transparent bit
                if (ibyte < andMask.length) {
                    andMask[ibyte] |= omask;
                }
            }
        }

        if (raster instanceof IntegerComponentRaster) {
            ficW = ((IntegerComponentRaster)raster).getScanlineStride();
        }
        setNativeIcon(((DataBufferInt)bimage.getRaster().getDataBuffer()).getData(),
                      andMask, ficW, raster.getWidth(), raster.getHeight());
    }

    void postEvent(AWTEvent event) {
        WToolkit.postEvent(WToolkit.targetToAppContext(target), event);
    }

    native void create();
    synchronized native void _dispose();

    /*
     * Updates/adds the icon in/to the system tray.
     * @param doUpdate if <code>true</code>, updates the icon,
     * otherwise, adds the icon
     */
    native void updateNativeIcon(boolean doUpdate);

    native void setNativeIcon(int[] rData, byte[] andMask, int nScanStride,
                              int width, int height);

    native void _displayMessage(String caption, String text, String messageType);

    class IconObserver implements ImageObserver {
        public boolean imageUpdate(Image image, int flags, int x, int y, int width, int height) {
            if (image != ((TrayIcon)target).getImage() || // if the image has been changed
                isDisposed())
            {
                return false;
            }
            if ((flags & (ImageObserver.FRAMEBITS | ImageObserver.ALLBITS |
                          ImageObserver.WIDTH | ImageObserver.HEIGHT)) != 0)
            {
                updateNativeImage(image);
            }
            return (flags & ImageObserver.ALLBITS) == 0;
        }
    }
}
