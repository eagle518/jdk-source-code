/*
 * @(#)PrintBandDescriptor.java	1.1 07/11/14
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

/** Encapsulating band image information for iterative printing. */

public class PrintBandDescriptor {
    
    /** The following info is to mark the state of each iterative band printing */
    private BufferedImage bandImage = null;
    private Graphics2D g2d = null;
    private int nextBandTop = 0;
    private boolean isLastBand = false;

    /** The following is band image data and coordinates for printing */
    private byte[] data = null;  // DIB data
    private int offset = 0;      // Image offset
    private int sx = 0;          // Source Coordinates
    private int sy = 0;  
    private int swidth = 0;
    private int sheight = 0;
    private int dx = 0;          // Destination Coordinates
    private int dy = 0;
    private int dwidth = 0;
    private int dheight = 0;

    public PrintBandDescriptor(BufferedImage bandImage,
                               Graphics2D g2d,
                               int nextBandTop, boolean isLastBand,
                               byte[] data, int offset,
                               int sx, int xy, int swidth, int sheight,
                               int dx, int dy, int dwidth, int dheight) {
        this.bandImage = bandImage;
        this.g2d = g2d;
        updateBandInfo(nextBandTop, isLastBand,
                       data, offset,
                       sx, xy, swidth, sheight,
                       dx, dy, dwidth, dheight);

    }

    public void updateBandInfo(int nextBandTop, boolean isLastBand,
                               byte[] data, int offset,
                               int sx, int xy, int swidth, int sheight,
                               int dx, int dy, int dwidth, int dheight) {
        this.nextBandTop = nextBandTop;
        this.isLastBand = isLastBand;
        this.data = data;
        this.offset = offset;
        this.sx = sx;
        this.sy = sy;
        this.swidth = swidth;
        this.sheight = sheight;
        this.dx = dx;
        this.dy = dy;
        this.dwidth = dwidth;
        this.dheight = dheight;
    }

    public BufferedImage getBandImage() {
        return bandImage;
    }

    public Graphics2D getG2D() {
        return g2d;
    }

    public int getNextBandTop() {
        return nextBandTop;
    }

    public boolean isLastBand() {
        return isLastBand;
    }

    public byte[] getData() {
        return data;
    }

    public int getOffset() {
        return offset;
    }

    public int getSrcX() {
        return sx;
    }

    public int getSrcY() {
        return sy;
    }

    public int getSrcWidth() {
        return swidth;
    }

    public int getSrcHeight() {
        return sheight;
    }

    public int getDestX() {
        return dx;
    }

    public int getDestY() {
        return dy;
    }

    public int getDestWidth() {
        return dwidth;
    }

    public int getDestHeight() {
        return dheight;
    }

}
