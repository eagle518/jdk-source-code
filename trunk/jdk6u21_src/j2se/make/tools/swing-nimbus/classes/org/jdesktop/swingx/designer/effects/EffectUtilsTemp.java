/*
 * @(#)EffectUtilsTemp.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.effects;

import java.awt.Composite;
import java.awt.CompositeContext;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;

/**
 * EffectUtilsTemp - effect utils methods that are not being used for now but we might want later
 *
 * @author Created by Jasper Potts (Jun 18, 2007)
 * @version 1.0
 */
public class EffectUtilsTemp {

    /**
     * Extract the alpha channel of a image into new greyscale buffered image
     *
     * @param src Must but INT_ARGB buffered image
     * @return new TYPE_BYTE_GRAY image of just the alpha channel
     */
    public static BufferedImage extractAlpha(BufferedImage src) {
        int w = src.getWidth();
        int h = src.getHeight();
        // extract image alpha channel as greyscale image
        final BufferedImage greyImg = new BufferedImage(w, h, BufferedImage.TYPE_BYTE_GRAY);
        Graphics2D g2 = greyImg.createGraphics();
        g2.setComposite(new Composite() {
            public CompositeContext createContext(ColorModel srcColorModel, ColorModel dstColorModel,
                                                  RenderingHints hints) {
                return new CompositeContext() {
                    public void dispose() {}

                    public void compose(Raster src, Raster dstIn, WritableRaster dstOut) {
                        int width = Math.min(src.getWidth(), dstIn.getWidth());
                        int height = Math.min(src.getHeight(), dstIn.getHeight());
                        int[] srcPixels = new int[width];
                        byte[] dstPixels = new byte[width];
                        for (int y = 0; y < height; y++) {
                            src.getDataElements(0, y, width, 1, srcPixels);
                            for (int x = 0; x < width; x++) {
                                dstPixels[x] = (byte) ((srcPixels[x] & 0xFF000000) >>> 24);
                            }
                            dstOut.setDataElements(0, y, width, 1, dstPixels);
                        }
                    }
                };
            }
        });
        g2.drawImage(src, 0, 0, null);
        g2.dispose();
        return greyImg;
    }

}
