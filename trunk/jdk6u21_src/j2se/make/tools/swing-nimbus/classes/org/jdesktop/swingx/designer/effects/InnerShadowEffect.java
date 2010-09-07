/*
 * @(#)InnerShadowEffect.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.effects;

import org.jdesktop.swingx.designer.paint.Matte;
import org.jdesktop.swingx.graphics.GraphicsUtilities;

import javax.swing.UIDefaults;
import java.awt.Color;
import java.awt.image.BufferedImage;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.util.Arrays;

/**
 * InnerShadowEffect
 *
 * @author Created by Jasper Potts (Jun 18, 2007)
 * @version 1.0
 */
public class InnerShadowEffect extends ShadowEffect {

    protected InnerShadowEffect() {}

    ;

    public InnerShadowEffect(UIDefaults uiDefaults) {
        color = new Matte(Color.BLACK, uiDefaults);
    }

    // =================================================================================================================
    // Effect Methods

    /**
     * Get the display name for this effect
     *
     * @return The user displayable name
     */
    public String getDisplayName() {
        return "Inner Shadow";
    }

    /**
     * Get the type of this effect, one of UNDER,BLENDED,OVER. UNDER means the result of apply effect should be painted
     * under the src image. BLENDED means the result of apply sffect contains a modified src image so just it should be
     * painted. OVER means the result of apply effect should be painted over the src image.
     *
     * @return The effect type
     */
    public Effect.EffectType getEffectType() {
        return Effect.EffectType.OVER;
    }

    /**
     * Apply the effect to the src image generating the result . The result image may or may not contain the source
     * image depending on what the effect type is.
     *
     * @param src The source image for applying the effect to
     * @param dst The dstination image to paint effect result into. If this is null then a new image will be created
     * @param w   The width of the src image to apply effect to, this allow the src and dst buffers to be bigger than
     *            the area the need effect applied to it
     * @param h   The height of the src image to apply effect to, this allow the src and dst buffers to be bigger than
     *            the area the need effect applied to it
     * @return The result of appl
     */
    public BufferedImage applyEffect(BufferedImage src, BufferedImage dst, int w, int h) {
        // calculate offset
        double trangleAngle = Math.toRadians(angle - 90);
        int offsetX = (int) (Math.sin(trangleAngle) * distance);
        int offsetY = (int) (Math.cos(trangleAngle) * distance);
        // clac expanded size
        int tmpOffX = offsetX + size;
        int tmpOffY = offsetY + size;
        int tmpW = w + offsetX + size + size;
        int tmpH = h + offsetY + size + size;
        // create tmp buffers
        int[] lineBuf = getTmpIntArray(w);
        byte[] srcAlphaBuf = getTmpByteArray1(tmpW * tmpH);
        Arrays.fill(srcAlphaBuf, (byte) 0xFF);
        byte[] tmpBuf1 = getTmpByteArray2(tmpW * tmpH);
        byte[] tmpBuf2 = getTmpByteArray3(tmpW * tmpH);
        // extract src image alpha channel and inverse and offset
        Raster srcRaster = src.getRaster();
        for (int y = 0; y < h; y++) {
            int dy = (y + tmpOffY);
            int offset = dy * tmpW;
            srcRaster.getDataElements(0, y, w, 1, lineBuf);
            for (int x = 0; x < w; x++) {
                int dx = x + tmpOffX;
                srcAlphaBuf[offset + dx] = (byte) ((255 - ((lineBuf[x] & 0xFF000000) >>> 24)) & 0xFF);
            }
        }
        // blur
        float[] kernel = EffectUtils.createGaussianKernel(size * 2);
        EffectUtils.blur(srcAlphaBuf, tmpBuf2, tmpW, tmpH, kernel, size * 2); // horizontal pass
        EffectUtils.blur(tmpBuf2, tmpBuf1, tmpH, tmpW, kernel, size * 2);// vertical pass
        //rescale
        float spread = Math.min(1 / (1 - (0.01f * this.spread)), 255);
        for (int i = 0; i < tmpBuf1.length; i++) {
            int val = (int) (((int) tmpBuf1[i] & 0xFF) * spread);
            tmpBuf1[i] = (val > 255) ? (byte) 0xFF : (byte) val;
        }
        // create color image with shadow color and greyscale image as alpha
        if (dst == null) dst = GraphicsUtilities.createCompatibleTranslucentImage(w, h);
        WritableRaster shadowRaster = dst.getRaster();
        int red = color.getRed(), green = color.getGreen(), blue = color.getBlue();
        for (int y = 0; y < h; y++) {
            int srcY = y + tmpOffY;
            int offset = srcY * tmpW;
            int shadowOffset = (srcY - offsetY) * tmpW;
            for (int x = 0; x < w; x++) {
                int srcX = x + tmpOffX;
                int origianlAlphaVal = 255 - ((int) srcAlphaBuf[offset + srcX] & 0xFF);
                int shadowVal = (int) tmpBuf1[shadowOffset + (srcX - offsetX)] & 0xFF;
                int alphaVal = Math.min(origianlAlphaVal, shadowVal);
                lineBuf[x] = ((byte) alphaVal & 0xFF) << 24 | red << 16 | green << 8 | blue;
            }
            shadowRaster.setDataElements(0, y, w, 1, lineBuf);
        }
        return dst;
    }
}
