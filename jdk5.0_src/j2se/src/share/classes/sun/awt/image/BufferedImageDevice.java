/*
 * @(#)BufferedImageDevice.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.GraphicsDevice;
import java.awt.GraphicsConfiguration;

public class BufferedImageDevice extends GraphicsDevice
{
    GraphicsConfiguration gc;

    public BufferedImageDevice(BufferedImageGraphicsConfig gc) {
        this.gc = gc;
    }

    /**
     * Returns the type of this <code>GraphicsDevice</code>.
     * @return the type of this <code>GraphicsDevice</code>, which can
     * either be TYPE_RASTER_SCREEN, TYPE_PRINTER or TYPE_IMAGE_BUFFER.
     * @see #TYPE_RASTER_SCREEN
     * @see #TYPE_PRINTER
     * @see #TYPE_IMAGE_BUFFER
     */
    public int getType() {
        return GraphicsDevice.TYPE_IMAGE_BUFFER;
    }

    /**
     * Returns the identification string associated with this 
     * <code>GraphicsDevice</code>.
     * @return a <code>String</code> that is the identification
     * of this <code>GraphicsDevice</code>.
     */
    public String getIDstring() {
        return ("BufferedImage");
    }
    
    /**
     * Returns all of the <code>GraphicsConfiguration</code>
     * objects associated with this <code>GraphicsDevice</code>.
     * @return an array of <code>GraphicsConfiguration</code>
     * objects that are associated with this 
     * <code>GraphicsDevice</code>.
     */
    public GraphicsConfiguration[] getConfigurations() {
        return new GraphicsConfiguration[] { gc };
    }

    /**
     * Returns the default <code>GraphicsConfiguration</code>
     * associated with this <code>GraphicsDevice</code>.
     * @return the default <code>GraphicsConfiguration</code>
     * of this <code>GraphicsDevice</code>.
     */
    public GraphicsConfiguration getDefaultConfiguration() {
        return gc;
    }
}
