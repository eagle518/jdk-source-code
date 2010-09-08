package sun.awt;

import java.awt.image.BufferedImage;

/**
 * This is a stub to enable GTK LAF on Windows.
 */
public abstract class UNIXToolkit extends SunToolkit
{
    public static final Object GTK_LOCK = new Object();

    /**
     * No native GTK -- return false to force GTKDefaultEngine
     */
    public static boolean checkGTK() {
        return false;
    }

    /**
     * Do nothing since GTK is never loaded
     */
    public static void unloadGTK() {
    }

    /**
     * Just a stub - nothing to do.
     */
    public BufferedImage getStockIcon(final int widgetType,
                                      final String stockId,
                                      final int iconSize,
                                      final int direction,
                                      final String detail) {
        return null;
    }
}
