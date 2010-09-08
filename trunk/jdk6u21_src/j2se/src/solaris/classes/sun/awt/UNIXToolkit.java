/*
 * @(#)UNIXToolkit.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import java.awt.RenderingHints;
import static java.awt.RenderingHints.*;
import java.awt.color.ColorSpace;
import java.awt.image.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.security.action.GetIntegerAction;
import com.sun.java.swing.plaf.gtk.GTKConstants.TextDirection;
import sun.java2d.opengl.OGLRenderQueue;
import java.lang.reflect.InvocationTargetException;

public abstract class UNIXToolkit extends SunToolkit
{
    /** All calls into GTK should be synchronized on this lock */
    public static final Object GTK_LOCK = new Object();

    private static final int[] BAND_OFFSETS = { 0, 1, 2 }; 
    private static final int[] BAND_OFFSETS_ALPHA = { 0, 1, 2, 3 }; 
    private static final int DEFAULT_DATATRANSFER_TIMEOUT = 10000;

    private static Boolean nativeGTKAvailable = null;
    private BufferedImage tmpImage = null;

    public static int getDatatransferTimeout() {
        Integer dt = (Integer)AccessController.doPrivileged(
                new GetIntegerAction("sun.awt.datatransfer.timeout"));
        if (dt == null || dt <= 0) {
            return DEFAULT_DATATRANSFER_TIMEOUT;
        } else {
            return dt;
        }
    }

    /**
     * Load GTK libraries if necessary
     */
    public static boolean checkGTK() {
        synchronized (GTK_LOCK) {
            if (nativeGTKAvailable == null) {
                if (load_gtk()) {
                    nativeGTKAvailable = Boolean.TRUE;
                } else {
                    unloadGTK();
                    nativeGTKAvailable = Boolean.FALSE;
                }
            }
        }
        return nativeGTKAvailable.booleanValue();
    }

    /**
     * Unload GTK libraries if they are loaded
     */
    public static void unloadGTK() {
        synchronized (GTK_LOCK) {
            if (nativeGTKAvailable == Boolean.TRUE) {
                unload_gtk();
            }
            nativeGTKAvailable = null;
        }
    }

    /**
     * Overridden to handle GTK icon loading
     */
    protected Object lazilyLoadDesktopProperty(String name) {
        if (name.startsWith("gtk.icon.")) {
            return lazilyLoadGTKIcon(name);
        }
	return super.lazilyLoadDesktopProperty(name);
    }

    /**
     * Load a native Gtk stock icon.  
     *
     * @param longname a desktop property name. This contains icon name, size
     *        and orientation, e.g. <code>"gtk.icon.gtk-add.4.rtl"</code>
     * @return an <code>Image</code> for the icon, or <code>null</code> if the
     *         icon could not be loaded
     */
    protected Object lazilyLoadGTKIcon(String longname) {
        // Check if we have already loaded it.
        Object result = desktopProperties.get(longname);
        if (result != null) {
            return result;
        }

        // We need to have at least gtk.icon.<stock_id>.<size>.<orientation>
        String str[] = longname.split("\\.");
        if (str.length != 5) {
            return null;
        }

        // Parse out the stock icon size we are looking for.
        int size = 0;
        try {
            size = Integer.parseInt(str[3]);
        } catch (NumberFormatException nfe) {
            return null;
        }

        // Direction.
        TextDirection dir = ("ltr".equals(str[4]) ? TextDirection.LTR :
                                                    TextDirection.RTL);
        
        // Load the stock icon.
        BufferedImage img = getStockIcon(-1, str[2], size, dir.ordinal(), null);
        if (img != null) {
            // Create the desktop property for the icon.
            setDesktopProperty(longname, img);
        }
        return img;
    }

    /**
     * Returns a BufferedImage which contains the Gtk icon requested.  If no
     * such icon exists or an error occurs loading the icon the result will
     * be null.
     *
     * @param filename
     * @return The icon or null if it was not found or loaded.
     */
    public BufferedImage getGTKIcon(final String filename) {
        if (!checkGTK()) {
            return null;

        } else {
            // Call the native method to load the icon.
            synchronized (GTK_LOCK) {
                if (!load_gtk_icon(filename)) {
                    tmpImage = null;
                }
            }
        }
        // Return local image the callback loaded the icon into.
        return tmpImage;
    }

    /**
     * Returns a BufferedImage which contains the Gtk stock icon requested.
     * If no such stock icon exists the result will be null.
     *
     * @param widgetType one of WidgetType values defined in GTKNativeEngine or
     * -1 for system default stock icon.
     * @param stockId String which defines the stock id of the gtk item.
     * For a complete list reference the API at www.gtk.org for StockItems.
     * @param iconSize One of the GtkIconSize values defined in GTKConstants 
     * @param textDirection One of the TextDirection values defined in
     * GTKConstants
     * @param detail Render detail that is passed to the native engine (feel
     * free to pass null)
     * @return The stock icon or null if it was not found or loaded.
     */
    public BufferedImage getStockIcon(final int widgetType, final String stockId, 
                                final int iconSize, final int direction, 
                                final String detail) {
        if (!checkGTK()) {
            return null;

        } else {
            // Call the native method to load the icon.
            synchronized (GTK_LOCK) {
                if (!load_stock_icon(widgetType, stockId, iconSize, direction, detail)) {
                    tmpImage = null;
                }
            }
        }
        // Return local image the callback loaded the icon into.
        return tmpImage;  // set by loadIconCallback
    }
                
    /**
     * This method is used by JNI as a callback from load_stock_icon.
     * Image data is passed back to us via this method and loaded into the
     * local BufferedImage and then returned via getStockIcon.
     *
     * Do NOT call this method directly.
     */ 
    public void loadIconCallback(byte[] data, int width, int height,
            int rowStride, int bps, int channels, boolean alpha) {
        // Reset the stock image to null.
        tmpImage = null;

        // Create a new BufferedImage based on the data returned from the
        // JNI call.
        DataBuffer dataBuf = new DataBufferByte(data, (rowStride * height));
        // Maybe test # channels to determine band offsets?
        WritableRaster raster = Raster.createInterleavedRaster(dataBuf,
                width, height, rowStride, channels,
                (alpha ? BAND_OFFSETS_ALPHA : BAND_OFFSETS), null);
        ColorModel colorModel = new ComponentColorModel(
                ColorSpace.getInstance(ColorSpace.CS_sRGB), alpha, false,
                ColorModel.TRANSLUCENT, DataBuffer.TYPE_BYTE);

        // Set the local image so we can return it later from
        // getStockIcon().
        tmpImage = new BufferedImage(colorModel, raster, false, null);
    }

    private static native boolean load_gtk();
    private static native boolean unload_gtk();
    private native boolean load_gtk_icon(String filename);
    private native boolean load_stock_icon(int widget_type, String stock_id,
            int iconSize, int textDirection, String detail);

    private native void nativeSync();

    public void sync() {
        // flush the X11 buffer
        nativeSync();
        // now flush the OGL pipeline (this is a no-op if OGL is not enabled)
        OGLRenderQueue.sync();
    }
   
    /*
     * This returns the value for the desktop property "awt.font.desktophints"
     * It builds this by querying the Gnome desktop properties to return
     * them as platform independent hints.
     * This requires that the Gnome properties have already been gathered.
     */
    public static final String FONTCONFIGAAHINT = "fontconfig/Antialias";
    protected RenderingHints getDesktopAAHints() {

        Object aaValue = getDesktopProperty("gnome.Xft/Antialias");

        if (aaValue == null) {
            /* On a KDE desktop running KWin the rendering hint will
             * have been set as property "fontconfig/Antialias".
             * No need to parse further in this case.
             */
            aaValue = getDesktopProperty(FONTCONFIGAAHINT);
            if (aaValue != null) {
               return new RenderingHints(KEY_TEXT_ANTIALIASING, aaValue); 
            } else {
                 return null; // no Gnome or KDE Desktop properties available.
            }
        }

        /* 0 means off, 1 means some ON. What would any other value mean?
         * If we require "1" to enable AA then some new value would cause
         * us to default to "OFF". I don't think that's the best guess.
         * So if its !=0 then lets assume AA.
         */
        boolean aa = Boolean.valueOf(((aaValue instanceof Number) &&
                                      ((Number)aaValue).intValue() != 0));
        Object aaHint;
        if (aa) {
            String subpixOrder =
                (String)getDesktopProperty("gnome.Xft/RGBA");

            if (subpixOrder == null || subpixOrder.equals("none")) {
                aaHint = VALUE_TEXT_ANTIALIAS_ON;
            } else if (subpixOrder.equals("rgb")) {
                aaHint = VALUE_TEXT_ANTIALIAS_LCD_HRGB;
            } else if (subpixOrder.equals("bgr")) {
                aaHint = VALUE_TEXT_ANTIALIAS_LCD_HBGR;
            } else if (subpixOrder.equals("vrgb")) {
                aaHint = VALUE_TEXT_ANTIALIAS_LCD_VRGB;
            } else if (subpixOrder.equals("vbgr")) {
                aaHint = VALUE_TEXT_ANTIALIAS_LCD_VBGR;
            } else {
                /* didn't recognise the string, but AA is requested */
                aaHint = VALUE_TEXT_ANTIALIAS_ON;
            }
        } else {
            aaHint = VALUE_TEXT_ANTIALIAS_DEFAULT;
        }
        return new RenderingHints(KEY_TEXT_ANTIALIASING, aaHint);
    }
}
