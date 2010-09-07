/*
 * @(#)MCustomCursor.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.X11CustomCursor;
import sun.awt.CustomCursor;
import java.awt.*;
import java.awt.image.*;
import sun.awt.image.ImageRepresentation;

public class MCustomCursor extends X11CustomCursor {

    public MCustomCursor(Image cursor, Point hotSpot, String name) 
            throws IndexOutOfBoundsException {
        super(cursor, hotSpot, name);
    }
    /**
     * Returns the supported cursor size
     */
    public static Dimension getBestCursorSize(
        int preferredWidth, int preferredHeight) {

        // Fix for bug 4212593 The Toolkit.createCustomCursor does not 
        //                     check absence of the image of cursor 
        // We use XQueryBestCursor which accepts unsigned ints to obtain
        // the largest cursor size that could be dislpayed 
        Dimension d = new Dimension(Math.abs(preferredWidth), Math.abs(preferredHeight));

        queryBestCursor(d);
        return d;
    }

    private static native void queryBestCursor(Dimension d);

    protected native void createCursor(byte[] xorMask, byte[] andMask, 
                                     int width, int height, 
                                     int fcolor, int bcolor, 
                                     int xHotSpot, int yHotSpot);

    static {
        cacheInit();
    }

    private native static void cacheInit();
}
