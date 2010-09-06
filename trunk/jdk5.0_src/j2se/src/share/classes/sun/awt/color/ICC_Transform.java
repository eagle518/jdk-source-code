/*
 * @(#)ICC_Transform.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
    Contains:    ICC transform class

            Created by gbp, October 1, 1997

    Copyright:    (C) 1997 by Eastman Kodak Company, all rights reserved.

    @(#)ICC_Transform.java	1.2    11/06/97
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/


package sun.awt.color;

import java.awt.color.ICC_Profile;
import java.awt.color.ProfileDataException;
import java.awt.color.CMMException;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.awt.image.ColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.SampleModel;
import java.awt.image.DataBuffer;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.ComponentSampleModel;


public class ICC_Transform {
    long    ID;

    public static final int Any = -1;/* any rendering type, whichever is
                                        available */
                                     /* search order is icPerceptual,
                                        icRelativeColorimetric, icSaturation */

    /* Transform types */
    public static final int In = 1;
    public static final int Out = 2;
    public static final int Gamut = 3;
    public static final int Simulation = 4;

    /* the class initializer */
    static {
        if (ProfileDeferralMgr.deferring) {
            ProfileDeferralMgr.activateProfiles();
        }
    }

    /**
     * Constructs an empty ICC_Transform object.
     */
    public ICC_Transform ()
    {
    }


   /**
     * Constructs an ICC_Transform object corresponding to an ICC profile transform.
     */
    public ICC_Transform (    ICC_Profile profile,
                int renderType,
                int transformType)
    {
	if (profile == null) {
	    CMM.checkStatus(CMM.cmmStatBadProfile);
        }
        CMM.checkStatus(CMM.cmmGetTransform (profile, renderType, transformType, this));
    }



    /**
     * Constructs an ICC_Transform object from a list of ICC_Transform objects.
     */
    public ICC_Transform (    ICC_Transform[]    transforms)
    {
    int        cmmStatus;
    long[]    transformIDs;
    int        nTransforms, i1;

        nTransforms = transforms.length;

        transformIDs = new long [nTransforms];

        /* put transform IDs into an array */
        for (i1 = 0; i1 < nTransforms; i1++) {
            transformIDs [i1] = transforms[i1].ID;
        }

        cmmStatus = CMM.cmmCombineTransforms (transformIDs, this);

        if ((cmmStatus != CMM.cmmStatSuccess) || (ID == 0)) {
            throw new ProfileDataException ("Invalid profile sequence");
        }
    }



    /**
     * returns the ID of an ICC_Transform object.
     */
    long getID ()
    {
        return ID;
    }


    /**
     * Frees the resources associated with an ICC_Transform object.
     */
    public void finalize ()
    {
        CMM.checkStatus (CMM.cmmFreeTransform (ID));
    }


    /* get the number of input components of a transform */
    public int
        getNumInComponents ()
    {
    int[]    nComps;
    
        nComps = new int [2];

        CMM.checkStatus (CMM.cmmGetNumComponents (ID, nComps));

        return nComps [0];
    }



    /* get the number of output components of a transform */
    public int
        getNumOutComponents ()
    {
    int[]    nComps;
    
        nComps = new int [2];

        CMM.checkStatus (CMM.cmmGetNumComponents (ID, nComps));

        return nComps [1];
    }



    /* color convert a BufferedImage */
    public void colorConvert (BufferedImage src, BufferedImage dst) {
        CMMImageLayout srcIL, dstIL;

        srcIL = getImageLayout(src);
        if (srcIL != null) {
            dstIL = getImageLayout(dst);
            if (dstIL != null) {
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));
                return;
            }
        }

        // Can't pass src and dst directly to CMM, so process per scanline
        Raster srcRas = src.getRaster();
        WritableRaster dstRas = dst.getRaster();
        ColorModel srcCM = src.getColorModel();
        ColorModel dstCM = dst.getColorModel();
        int w = src.getWidth();
        int h = src.getHeight();
        int srcNumComp = srcCM.getNumColorComponents();
        int dstNumComp = dstCM.getNumColorComponents();
        int precision = 8;
        float maxNum = 255.0f;
        for (int i = 0; i < srcNumComp; i++) {
            if (srcCM.getComponentSize(i) > 8) {
                 precision = 16;
                 maxNum = 65535.0f;
             }
        }
        for (int i = 0; i < dstNumComp; i++) {
            if (dstCM.getComponentSize(i) > 8) {
                 precision = 16;
                 maxNum = 65535.0f;
             }
        }
        float[] srcMinVal = new float[srcNumComp];
        float[] srcInvDiffMinMax = new float[srcNumComp];
        ColorSpace cs = srcCM.getColorSpace();
        for (int i = 0; i < srcNumComp; i++) {
            srcMinVal[i] = cs.getMinValue(i);
            srcInvDiffMinMax[i] = maxNum / (cs.getMaxValue(i) - srcMinVal[i]);
        }
        cs = dstCM.getColorSpace();
        float[] dstMinVal = new float[dstNumComp];
        float[] dstDiffMinMax = new float[dstNumComp];
        for (int i = 0; i < dstNumComp; i++) {
            dstMinVal[i] = cs.getMinValue(i);
            dstDiffMinMax[i] = (cs.getMaxValue(i) - dstMinVal[i]) / maxNum;
        }
        boolean dstHasAlpha = dstCM.hasAlpha();
        boolean needSrcAlpha = srcCM.hasAlpha() && dstHasAlpha;
        float[] dstColor;
        if (dstHasAlpha) {
            dstColor = new float[dstNumComp + 1];
        } else {
            dstColor = new float[dstNumComp];
        }
        if (precision == 8) {
            byte[] srcLine = new byte[w * srcNumComp];
            byte[] dstLine = new byte[w * dstNumComp];
            Object pixel;
            float[] color;
            float[] alpha = null;
            if (needSrcAlpha) {
                alpha = new float[w];
            }
            int idx;
            pelArrayInfo thePelArrayInfo =
                new pelArrayInfo(this, srcLine, dstLine); 
            srcIL = new CMMImageLayout(srcLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nSrc);
            dstIL = new CMMImageLayout(dstLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nDest);
            // process each scanline
            for (int y = 0; y < h; y++) {
                // convert src scanline
                pixel = null;
                color = null;
                idx = 0;
                for (int x = 0; x < w; x++) {
                    pixel = srcRas.getDataElements(x, y, pixel);
                    color = srcCM.getNormalizedComponents(pixel, color, 0);
                    for (int i = 0; i < srcNumComp; i++) {
                        srcLine[idx++] = (byte)
                            ((color[i] - srcMinVal[i]) * srcInvDiffMinMax[i] +
                             0.5f);
                    }
                    if (needSrcAlpha) {
                        alpha[x] = color[srcNumComp];
                    }
                }
                // color convert srcLine to dstLine
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));
                // convert dst scanline
                pixel = null;
                idx = 0;
                for (int x = 0; x < w; x++) {
                    for (int i = 0; i < dstNumComp; i++) {
                        dstColor[i] = ((float) (dstLine[idx++] & 0xff)) *
                                      dstDiffMinMax[i] + dstMinVal[i];
                    }
                    if (needSrcAlpha) {
                        dstColor[dstNumComp] = alpha[x];
                    } else if (dstHasAlpha) {
                        dstColor[dstNumComp] = 1.0f;
                    }
                    pixel = dstCM.getDataElements(dstColor, 0, pixel);
                    dstRas.setDataElements(x, y, pixel);
                }
            }
        } else {
            short[] srcLine = new short[w * srcNumComp];
            short[] dstLine = new short[w * dstNumComp];
            Object pixel;
            float[] color;
            float[] alpha = null;
            if (needSrcAlpha) {
                alpha = new float[w];
            }
            int idx;
            pelArrayInfo thePelArrayInfo =
                new pelArrayInfo(this, srcLine, dstLine); 
            srcIL = new CMMImageLayout(srcLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nSrc);
            dstIL = new CMMImageLayout(dstLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nDest);
            // process each scanline
            for (int y = 0; y < h; y++) {
                // convert src scanline
                pixel = null;
                color = null;
                idx = 0;
                for (int x = 0; x < w; x++) {
                    pixel = srcRas.getDataElements(x, y, pixel);
                    color = srcCM.getNormalizedComponents(pixel, color, 0);
                    for (int i = 0; i < srcNumComp; i++) {
                        srcLine[idx++] = (short)
                            ((color[i] - srcMinVal[i]) * srcInvDiffMinMax[i] +
                             0.5f);
                    }
                    if (needSrcAlpha) {
                        alpha[x] = color[srcNumComp];
                    }
                }
                // color convert srcLine to dstLine
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));
                // convert dst scanline
                pixel = null;
                idx = 0;
                for (int x = 0; x < w; x++) {
                    for (int i = 0; i < dstNumComp; i++) {
                        dstColor[i] = ((float) (dstLine[idx++] & 0xffff)) *
                                      dstDiffMinMax[i] + dstMinVal[i];
                    }
                    if (needSrcAlpha) {
                        dstColor[dstNumComp] = alpha[x];
                    } else if (dstHasAlpha) {
                        dstColor[dstNumComp] = 1.0f;
                    }
                    pixel = dstCM.getDataElements(dstColor, 0, pixel);
                    dstRas.setDataElements(x, y, pixel);
                }
            }
        }
        
    }


    /* Get a CMMImageLayout structure for a BufferedImage.  Returns null
     * if we don't determine that this BufferedImage has a structure
     * that can be passed directly to the CMM.  Not an exhaustive
     * determination, but should handle the important cases.  If
     * this method returns null for src or dst of a color conversion
     * operation, a slower scanline by scanline approach is used.
     */
    private CMMImageLayout getImageLayout(BufferedImage img) {

        switch (img.getType()) {
            case BufferedImage.TYPE_INT_RGB:
            case BufferedImage.TYPE_INT_ARGB:
            case BufferedImage.TYPE_INT_BGR:
                return new CMMImageLayout(img);
            case BufferedImage.TYPE_3BYTE_BGR:
            case BufferedImage.TYPE_4BYTE_ABGR: {
                ComponentColorModel cm =
                    (ComponentColorModel) img.getColorModel();
                if ((cm.getClass() == ComponentColorModel.class) ||
                    (checkMinMaxScaling(cm))) {
                    return new CMMImageLayout(img);
                }
                return null;
            }
            case BufferedImage.TYPE_BYTE_GRAY: {
                ComponentColorModel cm =
                    (ComponentColorModel) img.getColorModel();
                if (cm.getComponentSize(0) != 8) {
                    return null;
                }
                if ((cm.getClass() == ComponentColorModel.class) ||
                    (checkMinMaxScaling(cm))) {
                    return new CMMImageLayout(img);
                }
                return null;
            }
            case BufferedImage.TYPE_USHORT_GRAY: {
                ComponentColorModel cm =
                    (ComponentColorModel) img.getColorModel();
                if (cm.getComponentSize(0) != 16) {
                    return null;
                }
                if ((cm.getClass() == ComponentColorModel.class) ||
                    (checkMinMaxScaling(cm))) {
                    return new CMMImageLayout(img);
                }
                return null;
            }
        }
        ColorModel cm = img.getColorModel();
        if (cm instanceof DirectColorModel) {
            // must be RGB and have appropriate min/max scaling by
            // definition of DCM
            SampleModel sm = img.getSampleModel();
            if (!(sm instanceof SinglePixelPackedSampleModel)) {
                return null;
            }
            if (cm.getTransferType() != DataBuffer.TYPE_INT) {
                return null;
            }
            if (cm.hasAlpha() && cm.isAlphaPremultiplied()) {
                return null;
            }
            DirectColorModel dcm = (DirectColorModel) cm;
            // check that RGB masks are all 8-bit and byte aligned
            int rmask = dcm.getRedMask();
            int gmask = dcm.getGreenMask();
            int bmask = dcm.getBlueMask();
            int amask = dcm.getAlphaMask();
            int rPos, gPos, bPos, aPos;
            rPos = gPos = bPos = aPos = -1;
            int match = 0;
            int mustMatch = 3;
            if (amask != 0) {
                mustMatch = 4;
            }
            for (int i = 0, mask = 0xFF000000; i < 4; i++, mask >>>= 8) {
                if (rmask == mask) {
                    rPos = i;
                    match += 1;
                } else if (gmask == mask) {
                    gPos = i;
                    match += 1;
                } else if (bmask == mask) {
                    bPos = i;
                    match += 1;
                } else if (amask == mask) {
                    aPos = i;
                    match += 1;
                }
            }
            if (match != mustMatch) {
                return null;
            }
            return new CMMImageLayout(img, (SinglePixelPackedSampleModel) sm,
                                      rPos, gPos, bPos, aPos);
        } else if (cm instanceof ComponentColorModel) {
            SampleModel sm = img.getSampleModel();
            if (!(sm instanceof ComponentSampleModel)) {
                return null;
            }
            if (cm.hasAlpha() && cm.isAlphaPremultiplied()) {
                return null;
            }
            int nc = cm.getNumComponents();
            if (sm.getNumBands() != nc) {
                return null;
            }
            int transferType = cm.getTransferType();
            if (transferType == DataBuffer.TYPE_BYTE) {
                for (int i = 0; i < nc; i++) {
                    if (cm.getComponentSize(i) != 8) {
                        return null;
                    }
                }
            } else if (transferType == DataBuffer.TYPE_USHORT) {
                for (int i = 0; i < nc; i++) {
                    if (cm.getComponentSize(i) != 16) {
                        return null;
                    }
                }
            } else {
                return null;
            }
            ComponentColorModel ccm = (ComponentColorModel) cm;
            if ((ccm.getClass() == ComponentColorModel.class) ||
                (checkMinMaxScaling(ccm))) {
                return new CMMImageLayout(img, (ComponentSampleModel) sm);
            }
            return null;
        } else {
            // REMIND: could provide additional optimized cases for
            // PackedColorModels, MultiPixelPackedSampleModels,
            // 565 and 555 DirectColorModels, 12-bit Grayscale images,
            // and single band ComponentColorModel images with a
            // SinglePixelPackedSampleModel
            return null;
        }
    }


    private boolean checkMinMaxScaling(ComponentColorModel cm) {
        float[] lowVal, highVal;
        float divisor;
        int numComponents = cm.getNumComponents();
        int numColorComponents = cm.getNumColorComponents();
        int[] nBits = cm.getComponentSize();
        boolean supportsAlpha = cm.hasAlpha();

        switch (cm.getTransferType()) {
        case DataBuffer.TYPE_BYTE:
            {
                byte[] bpixel = new byte[numComponents];
                for (int i = 0; i < numColorComponents; i++) {
                    bpixel[i] = 0;
                }
                if (supportsAlpha) {
                    bpixel[numColorComponents] =
                        (byte) ((1 << nBits[numColorComponents]) - 1);
                }
                lowVal = cm.getNormalizedComponents(bpixel, null, 0);
                for (int i = 0; i < numColorComponents; i++) {
                    bpixel[i] = (byte) ((1 << nBits[i]) - 1);
                }
                highVal = cm.getNormalizedComponents(bpixel, null, 0);
                divisor = 256.0f;
            }
            break;
        case DataBuffer.TYPE_USHORT:
            {
                short[] uspixel = new short[numComponents];
                for (int i = 0; i < numColorComponents; i++) {
                    uspixel[i] = 0;
                }
                if (supportsAlpha) {
                    uspixel[numColorComponents] =
                        (byte) ((1 << nBits[numColorComponents]) - 1);
                }
                lowVal = cm.getNormalizedComponents(uspixel, null, 0);
                for (int i = 0; i < numColorComponents; i++) {
                    uspixel[i] = (byte) ((1 << nBits[i]) - 1);
                }
                highVal = cm.getNormalizedComponents(uspixel, null, 0);
                divisor = 65536.0f;
            }
            break;
        default:
            return false;
        }
        ColorSpace cs = cm.getColorSpace();
        for (int i = 0; i < numColorComponents; i++) {
            float min = cs.getMinValue(i);
            float max = cs.getMaxValue(i);
            float threshold = (max - min) / divisor;
            min = min - lowVal[i];
            if (min < 0.0f) {
                min = -min;
            }
            max = max - highVal[i];
            if (max < 0.0f) {
                max = -max;
            }
            if ((min > threshold) || (max > threshold)) {
                return false;
            }
        }
        return true;
    }


    /* color convert a Raster when at least one of src and dst has a float
       or double transferType */
    public void colorConvert(Raster src, WritableRaster dst,
                             float[] srcMinVal, float[]srcMaxVal,
                             float[] dstMinVal, float[]dstMaxVal) {
        CMMImageLayout srcIL, dstIL;

        // Can't pass src and dst directly to CMM, so process per scanline
        SampleModel srcSM = src.getSampleModel();
        SampleModel dstSM = dst.getSampleModel();
        int srcTransferType = src.getTransferType();
        int dstTransferType = dst.getTransferType();
        boolean srcIsFloat, dstIsFloat;
        if ((srcTransferType == DataBuffer.TYPE_FLOAT) ||
            (srcTransferType == DataBuffer.TYPE_DOUBLE)) {
            srcIsFloat = true;
        } else {
            srcIsFloat = false;
        }
        if ((dstTransferType == DataBuffer.TYPE_FLOAT) ||
            (dstTransferType == DataBuffer.TYPE_DOUBLE)) {
            dstIsFloat = true;
        } else {
            dstIsFloat = false;
        }
        int w = src.getWidth();
        int h = src.getHeight();
        int srcNumBands = src.getNumBands();
        int dstNumBands = dst.getNumBands();
        float[] srcScaleFactor = new float[srcNumBands];
        float[] dstScaleFactor = new float[dstNumBands];
        float[] srcUseMinVal = new float[srcNumBands];
        float[] dstUseMinVal = new float[dstNumBands];
        for (int i = 0; i < srcNumBands; i++) {
            if (srcIsFloat) {
                srcScaleFactor[i] =  65535.0f / (srcMaxVal[i] - srcMinVal[i]);
                srcUseMinVal[i] = srcMinVal[i];
            } else {
                if (srcTransferType == DataBuffer.TYPE_SHORT) {
                    srcScaleFactor[i] = 65535.0f / 32767.0f;
                } else {
                    srcScaleFactor[i] = 65535.0f /
                        ((float) ((1 << srcSM.getSampleSize(i)) - 1));
                }
                srcUseMinVal[i] = 0.0f;
            }
        }
        for (int i = 0; i < dstNumBands; i++) {
            if (dstIsFloat) {
                dstScaleFactor[i] = (dstMaxVal[i] - dstMinVal[i]) / 65535.0f;
                dstUseMinVal[i] = dstMinVal[i];
            } else {
                if (dstTransferType == DataBuffer.TYPE_SHORT) {
                    dstScaleFactor[i] = 32767.0f / 65535.0f;
                } else {
                    dstScaleFactor[i] = 
                        ((float) ((1 << dstSM.getSampleSize(i)) - 1)) /
                        65535.0f;
                }
                dstUseMinVal[i] = 0.0f;
            }
        }
        int ys = src.getMinY();
        int yd = dst.getMinY();
        int xs, xd;
        float sample;
        short[] srcLine = new short[w * srcNumBands];
        short[] dstLine = new short[w * dstNumBands];
        int idx;
        pelArrayInfo thePelArrayInfo =
            new pelArrayInfo(this, srcLine, dstLine); 
        srcIL = new CMMImageLayout(srcLine, thePelArrayInfo.nPels,
                                   thePelArrayInfo.nSrc);
        dstIL = new CMMImageLayout(dstLine, thePelArrayInfo.nPels,
                                   thePelArrayInfo.nDest);
        // process each scanline
        for (int y = 0; y < h; y++, ys++, yd++) {
            // get src scanline
            xs = src.getMinX();
            idx = 0;
            for (int x = 0; x < w; x++, xs++) {
                for (int i = 0; i < srcNumBands; i++) {
                    sample = src.getSampleFloat(xs, ys, i);
                    srcLine[idx++] = (short)
                        ((sample - srcUseMinVal[i]) * srcScaleFactor[i] + 0.5f);
                }
            }

            // color convert srcLine to dstLine
            CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));

            // store dst scanline
            xd = dst.getMinX();
            idx = 0;
            for (int x = 0; x < w; x++, xd++) {
                for (int i = 0; i < dstNumBands; i++) {
                    sample = ((dstLine[idx++] & 0xffff) * dstScaleFactor[i]) +
                             dstUseMinVal[i];
                    dst.setSample(xd, yd, i, sample);
                }
            }
        }

    }


    /* color convert a Raster with byte, ushort, int, or short transferType */
    public void colorConvert(Raster src, WritableRaster dst) {
        CMMImageLayout srcIL, dstIL;

        srcIL = getImageLayout(src);
        if (srcIL != null) {
            dstIL = getImageLayout(dst);
            if (dstIL != null) {
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));
                return;
            }
        }

        // Can't pass src and dst directly to CMM, so process per scanline
        SampleModel srcSM = src.getSampleModel();
        SampleModel dstSM = dst.getSampleModel();
        int srcTransferType = src.getTransferType();
        int dstTransferType = dst.getTransferType();
        int w = src.getWidth();
        int h = src.getHeight();
        int srcNumBands = src.getNumBands();
        int dstNumBands = dst.getNumBands();
        int precision = 8;
        float maxNum = 255.0f;
        for (int i = 0; i < srcNumBands; i++) {
            if (srcSM.getSampleSize(i) > 8) {
                 precision = 16;
                 maxNum = 65535.0f;
             }
        }
        for (int i = 0; i < dstNumBands; i++) {
            if (dstSM.getSampleSize(i) > 8) {
                 precision = 16;
                 maxNum = 65535.0f;
             }
        }
        float[] srcScaleFactor = new float[srcNumBands];
        float[] dstScaleFactor = new float[dstNumBands];
        for (int i = 0; i < srcNumBands; i++) {
            if (srcTransferType == DataBuffer.TYPE_SHORT) {
                srcScaleFactor[i] = maxNum / 32767.0f;
            } else {
                srcScaleFactor[i] = maxNum /
                    ((float) ((1 << srcSM.getSampleSize(i)) - 1));
            }
        }
        for (int i = 0; i < dstNumBands; i++) {
            if (dstTransferType == DataBuffer.TYPE_SHORT) {
                dstScaleFactor[i] = 32767.0f / maxNum;
            } else {
                dstScaleFactor[i] = 
                    ((float) ((1 << dstSM.getSampleSize(i)) - 1)) / maxNum;
            }
        }
        int ys = src.getMinY();
        int yd = dst.getMinY();
        int xs, xd;
        int sample;
        if (precision == 8) {
            byte[] srcLine = new byte[w * srcNumBands];
            byte[] dstLine = new byte[w * dstNumBands];
            int idx;
            pelArrayInfo thePelArrayInfo =
                new pelArrayInfo(this, srcLine, dstLine); 
            srcIL = new CMMImageLayout(srcLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nSrc);
            dstIL = new CMMImageLayout(dstLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nDest);
            // process each scanline
            for (int y = 0; y < h; y++, ys++, yd++) {
                // get src scanline
                xs = src.getMinX();
                idx = 0;
                for (int x = 0; x < w; x++, xs++) {
                    for (int i = 0; i < srcNumBands; i++) {
                        sample = src.getSample(xs, ys, i);
                        srcLine[idx++] = (byte)
                            ((sample * srcScaleFactor[i]) + 0.5f);
                    }
                }

                // color convert srcLine to dstLine
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));

                // store dst scanline
                xd = dst.getMinX();
                idx = 0;
                for (int x = 0; x < w; x++, xd++) {
                    for (int i = 0; i < dstNumBands; i++) {
                        sample = (int) (((dstLine[idx++] & 0xff) *
                                         dstScaleFactor[i]) + 0.5f);
                        dst.setSample(xd, yd, i, sample);
                    }
                }
            }
        } else {
            short[] srcLine = new short[w * srcNumBands];
            short[] dstLine = new short[w * dstNumBands];
            int idx;
            pelArrayInfo thePelArrayInfo =
                new pelArrayInfo(this, srcLine, dstLine); 
            srcIL = new CMMImageLayout(srcLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nSrc);
            dstIL = new CMMImageLayout(dstLine, thePelArrayInfo.nPels,
                                       thePelArrayInfo.nDest);
            // process each scanline
            for (int y = 0; y < h; y++, ys++, yd++) {
                // get src scanline
                xs = src.getMinX();
                idx = 0;
                for (int x = 0; x < w; x++, xs++) {
                    for (int i = 0; i < srcNumBands; i++) {
                        sample = src.getSample(xs, ys, i);
                        srcLine[idx++] = (short)
                            ((sample * srcScaleFactor[i]) + 0.5f);
                    }
                }

                // color convert srcLine to dstLine
                CMM.checkStatus(CMM.cmmColorConvert(ID, srcIL, dstIL));

                // store dst scanline
                xd = dst.getMinX();
                idx = 0;
                for (int x = 0; x < w; x++, xd++) {
                    for (int i = 0; i < dstNumBands; i++) {
                        sample = (int) (((dstLine[idx++] & 0xffff) *
                                         dstScaleFactor[i]) + 0.5f);
                        dst.setSample(xd, yd, i, sample);
                    }
                }
            }
        }
        
    }


    /* Get a CMMImageLayout structure for a Raster.  Returns null
     * if we don't determine that this Raster has a structure
     * that can be passed directly to the CMM.  Not an exhaustive
     * determination, but handles many important cases.  If
     * this method returns null for src or dst of a color conversion
     * operation, a slower scanline by scanline approach is used.
     */
    private CMMImageLayout getImageLayout(Raster ras) {

        SampleModel sm = ras.getSampleModel();
        if (sm instanceof ComponentSampleModel) {
            int nc = ras.getNumBands();
            int transferType = sm.getTransferType();
            if (transferType == DataBuffer.TYPE_BYTE) {
                for (int i = 0; i < nc; i++) {
                    // REMIND: is this test necessary?
                    if (sm.getSampleSize(i) != 8) {
                        return null;
                    }
                }
            } else if (transferType == DataBuffer.TYPE_USHORT) {
                for (int i = 0; i < nc; i++) {
                    // REMIND: is this test necessary?
                    if (sm.getSampleSize(i) != 16) {
                        return null;
                    }
                }
            } else {
                return null;
            }
            return new CMMImageLayout(ras, (ComponentSampleModel) sm);
        } else {
            // REMIND: could provide additional optimized cases for
            // MultiPixelPackedSampleModels, SinglePixelPackedSampleModels,
            // 555 and 565 images, 12-bit component images
            return null;
        }
    }


    /* convert an array of colors in short format */
    /* each color is a contiguous set of array elements */
    /* the number of colors is (size of the array) / (number of input/output
       components */
    public short[] colorConvert(short[] src, short[] dest) {
        pelArrayInfo    thePelArrayInfo;
        CMMImageLayout    srcIL, destIL;
        short[]        result;
        int        cmmStatus;

        /* transform I/O correspond to arrays? */
        thePelArrayInfo = new pelArrayInfo (this, src, dest); 
        
        if (dest != null) {            /* check dest, make new if needed */
            result = dest;
        }
        else {
            result = new short [thePelArrayInfo.destSize];
        }

        /* make the source image layout */
        srcIL = new CMMImageLayout(src, thePelArrayInfo.nPels,
                                   thePelArrayInfo.nSrc);

        /* make the destination image layout */
        destIL = new CMMImageLayout(result, thePelArrayInfo.nPels,
                                    thePelArrayInfo.nDest);

        CMM.checkStatus(CMM.cmmColorConvert (ID, srcIL, destIL));

        return result;
    }



    /* convert an array of colors in byte format */
    /* each color is a contiguous set of array elements */
    /* the number of colors is (size of the array) / (number of input/output
       components */
    public byte[] colorConvert (byte[] src, byte[] dest) {
        pelArrayInfo    thePelArrayInfo;
        CMMImageLayout    srcIL, destIL;
        byte[]        result;
        int        cmmStatus;

        /* transform I/O correspond to arrays? */
        thePelArrayInfo = new pelArrayInfo (this, src, dest); 
        
        if (dest != null) {            /* check dest, make new if needed */
            result = dest;
        }
        else {
            result = new byte [thePelArrayInfo.destSize];
        }

        /* make the source image layout */
        srcIL = new CMMImageLayout(src, thePelArrayInfo.nPels,
                                   thePelArrayInfo.nSrc);

        /* make the destination image layout */
        destIL = new CMMImageLayout(result, thePelArrayInfo.nPels,
                                    thePelArrayInfo.nDest);

        CMM.checkStatus (CMM.cmmColorConvert (ID, srcIL, destIL));

        return result;
    }
}



/* validate input and output arrays which contain pixels */
/* return component information */
class pelArrayInfo {
    int    nPels;
    int    nSrc;
    int    srcSize;
    int    nDest;
    int    destSize;
    
    pelArrayInfo (    ICC_Transform    transform,
            int            ncol,
            float[]        src,
            float[]        dest)
    {
        nSrc = transform.getNumInComponents ();
        nDest = transform.getNumOutComponents ();

        nPels = ncol;
        srcSize = nPels * nSrc;
        destSize = nPels * nDest;

        if (srcSize > src.length) {
            throw new IllegalArgumentException ("Inconsistent pel structure");
        }
        
        if (dest != null) {            /* check dest, make new if needed */
            checkDest (dest.length);
        }
    }

    
    pelArrayInfo (    ICC_Transform    transform,
            short[]        src,
            short[]        dest)
    {
        srcSize = src.length;

        initInfo (transform);
        destSize = nPels * nDest;
        
        if (dest != null) {            /* check dest, make new if needed */
            checkDest (dest.length);
        }
    }

    
    pelArrayInfo (    ICC_Transform    transform,
            byte[]        src,
            byte[]        dest)
    {
        srcSize = src.length;

        initInfo (transform);
        destSize = nPels * nDest;
        
        if (dest != null) {            /* check dest, make new if needed */
            checkDest (dest.length);
        }
    }

    
    void    initInfo (    ICC_Transform    transform)
    {
        nSrc = transform.getNumInComponents ();    /* get # of in and out components */
        nDest = transform.getNumOutComponents ();

        nPels = srcSize / nSrc;            /* calculate # of pels */

        if ((nPels * nSrc) != srcSize) {    /* only complete pels allowed */
            throw new IllegalArgumentException ("Inconsistent pel structure");
        }
    }

    
    void    checkDest (int length)
    {
        if (destSize > length) {
            throw new IllegalArgumentException ("Inconsistent pel structure");
        }
    }
    
}
