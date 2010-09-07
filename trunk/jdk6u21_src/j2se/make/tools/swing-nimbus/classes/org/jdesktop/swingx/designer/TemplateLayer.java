/*
 * @(#)TemplateLayer.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.swingx.designer.effects.Effect;

import javax.imageio.ImageIO;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.lang.ref.SoftReference;

/**
 * TemplateLayer
 *
 * @author Created by Jasper Potts (Jul 2, 2007)
 * @version 1.0
 */
public class TemplateLayer extends Layer {

    private String fileName;
    private transient SoftReference<BufferedImage> imgRef = null;

    public TemplateLayer() {
        type = LayerType.template;
    }

    public TemplateLayer(String fileName, BufferedImage templateImage) {
        super("Template");
        this.fileName = fileName;
        type = LayerType.template;
        if (templateImage != null) {
            imgRef = new SoftReference<BufferedImage>(templateImage);
        }
    }

    // =================================================================================================================
    // Methods

    public String getName() {
        return super.getName();
    }

    /**
     * template layers are always locked
     *
     * @return <code>true</code>
     */
    public boolean isLocked() {
        return true;
    }

    public void add(SimpleShape shape) {
        throw new IllegalStateException("Template layers can't contain shapes");
    }

    public void addEffect(Effect effect) {
        throw new IllegalStateException("Template layers can't contain effects");
    }

    public void addLayer(int i, Layer layer) {
        throw new IllegalStateException("Template layers can't contain sub layers");
    }

    public void addLayer(Layer layer) {
        throw new IllegalStateException("Template layers can't contain sub layers");
    }

    public void paint(Graphics2D g2, double pixelSize) {
        if (isVisible()) {
            BufferedImage img = getTemplateImage();
            if (img != null) g2.drawImage(img, 0, 0, null);
        }
    }


    public Image getBuffer(GraphicsConfiguration graphicsConfiguration) {
        return getTemplateImage();
    }

    public BufferedImage getTemplateImage() {
        BufferedImage img = null;
        if (imgRef == null || (img = imgRef.get()) == null) {

            // can not access canvas
            final File templateImgFile = new File(getCanvas().getTemplatesDir(), fileName);
            System.out.println("templateImgFile = " + templateImgFile.getAbsolutePath());
            System.out.println("templateImgFile.exists = " + templateImgFile.exists());
            try {
                img = ImageIO.read(templateImgFile);
                imgRef = new SoftReference<BufferedImage>(img);
            } catch (IOException e) {
                e.printStackTrace();
                // create error image
                img = new BufferedImage(getCanvas().getSize().width, getCanvas().getSize().height,
                        BufferedImage.TYPE_INT_RGB);
                Graphics2D g2 = img.createGraphics();
                g2.setColor(Color.RED);
                g2.fillRect(0, 0, img.getWidth(), img.getHeight());
                g2.setColor(Color.WHITE);
                g2.setFont(g2.getFont().deriveFont(8f));
                FontMetrics fontMetrics = g2.getFontMetrics();
                Rectangle2D stringBounds = fontMetrics.getStringBounds("Missing Image", g2);
                int offsetX = (int) ((img.getWidth() - stringBounds.getWidth()) / 2d);
                int offsetY = (int) (((img.getHeight() - stringBounds.getHeight()) / 2d) - stringBounds.getY());
                g2.drawString("Missing Image", offsetX, offsetY);
                g2.dispose();
                imgRef = new SoftReference<BufferedImage>(img);
            }
        }
        return img;
    }
}
