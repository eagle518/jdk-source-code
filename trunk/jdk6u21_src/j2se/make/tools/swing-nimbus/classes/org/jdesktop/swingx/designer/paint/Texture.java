/*
 * @(#)Texture.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package org.jdesktop.swingx.designer.paint;

import java.awt.Paint;
import java.awt.TexturePaint;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;

/**
 * Represents a TexturePaint.
 *
 * @author rbair
 */
public class Texture extends PaintModel {
    private static final Rectangle2D RECT = new Rectangle2D.Double(0, 0, 1, 1);
    private BufferedImage img;

    public Texture() {
    }

    public PaintControlType getPaintControlType() {
        return PaintControlType.control_rect;
    }

    public void setImage(BufferedImage img) {
        BufferedImage old = this.img;
        this.img = img;
        firePropertyChange("paint", old, this.img);
        firePropertyChange("image", old, this.img);
    }

    public final BufferedImage getImage() {
        return img;
    }

    public Paint getPaint() {
        return new TexturePaint(img, RECT);
    }


    public Texture clone() {
        Texture newTexture = new Texture();
        newTexture.img = this.img;
        return newTexture;
    }
}
