/*
 * @(#)CMMImageLayout.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
    Contains:    image layout description class

            Created by gbp, October 16, 1997

    Copyright:    (C) 1997 by Eastman Kodak Company, all rights reserved.

    File: CMMImageLayout.java  @(#)CMMImageLayout.java	1.2    11/06/97
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/


package sun.awt.color;

import java.awt.image.BufferedImage;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.ComponentSampleModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferUShort;
import java.awt.image.DataBufferInt;
import java.awt.image.ColorModel;
import sun.awt.image.ByteComponentRaster;
import sun.awt.image.ShortComponentRaster;
import sun.awt.image.IntegerComponentRaster;


/* standard image description for communication with the CMM */
class CMMImageLayout {

    /* define image layout base so that BufferedImage.TYPE_INT_ARGB, etc.
       can be used directly */

    private static final int typeBase = 256;

    static public final int typeComponentUByte = typeBase + 0;
    /* component organization, each is 8 bits */

    static public final int typeComponentUShort12 = typeBase + 1;
    /* component organization, each is the low 12 bits of UShort */

    static public final int typeComponentUShort = typeBase + 2;
    /* component organization, each is 16 bits */

    static public final int typePixelUByte = typeBase + 3;
    /* pixel organization, each component is 8 bits */

    static public final int typePixelUShort12 = typeBase + 4;
    /* pixel organization, each component is the low 12 bits of UShort */

    static public final int typePixelUShort = typeBase + 5;
    /* pixel organization, each component is 16 bits */

    static public final int typeShort555 = typeBase + 6;
    /* red is 14-10, green is 9-5, blue is 4-0 */

    static public final int typeShort565 = typeBase + 7;
    /* red is 15-11, green is 10-5, blue is 4-0 */

    static public final int typeInt101010 = typeBase + 8;
    /* red is 29-20, green is 19-10, blue is 9-0 */

    static public final int typeIntRGBPacked = typeBase + 9;
    /* DCM/SPPSM images with arbitrary 8-bit byte-aligned RGB masks */

    public int    Type;            /* component format */
    public int    NumCols;         /* # of columns */
    public int    NumRows;         /* # of rows */
    public int    OffsetColumn;    /* byte offset to next column */
    public int    OffsetRow;       /* byte offset to next row */
    public int    NumChannels;     /* # of discrete components */
    public Object[]    chanData;   /* component arrays.  This will include
                                      an array containing alpha samples, if
                                      there is an alpha channel. */
    public int[]  DataOffsets;     /* offset(s) in bytes from the beginning
                                      of the array(s) to the element in
                                      which the corresponding sample
                                      of the first scanline is stored. */
    public int[]  sampleInfo;      /* optional format-specific additional info
                                      to locate a sample within an array
                                      element, e.g. a byte within an int or
                                      a bit field within a short */

    /* construct a layout for a byte array of 8-bit unsigned
       color components
     */
    public CMMImageLayout(byte[] img, int nPels, int nComps) {
        Type = typeComponentUByte;

        chanData = new Object [nComps];
        DataOffsets = new int[nComps];

        NumCols = nPels;                       /* # of columns */
        NumRows = 1;                           /* # of rows */
        OffsetColumn = nComps;                 /* bytes to next column */
        OffsetRow = NumCols * OffsetColumn;    /* bytes to next row */
        NumChannels = nComps;                  /* # of discrete channels */
        for (int i = 0; i < nComps; i++) {
            chanData[i] = img;
            DataOffsets[i] = i;
        }
    }



    /* construct a layout for a short array of 16-bit unsigned
       color components
     */
    public CMMImageLayout(short[] img, int nPels, int nComps) {
        Type = typeComponentUShort;

        chanData = new Object [nComps];
        DataOffsets = new int[nComps];

        NumCols = nPels;                       /* # of columns */
        NumRows = 1;                           /* # of rows */
        OffsetColumn = nComps * 2;             /* bytes to next column */
        OffsetRow = NumCols * OffsetColumn;    /* bytes to next row */
        NumChannels = nComps;                  /* # of discrete channels */
        for (int i = 0; i < nComps; i++) {
            chanData[i] = img;
            DataOffsets[i] = i * 2;
        }
    }



    /* convert a BufferedImage into a common form which is easier for
       the native cmm to use
     */
    public CMMImageLayout (BufferedImage theImage) {
        int nc;
        IntegerComponentRaster intRaster;
        ShortComponentRaster shortRaster;
        ByteComponentRaster byteRaster;
        int offset;
        Object dataArray;

        Type = theImage.getType();

        /* System.out.println ("Type = " + Type); */

        switch (Type) {
        case BufferedImage.TYPE_INT_RGB:
        case BufferedImage.TYPE_INT_ARGB:
        case BufferedImage.TYPE_INT_BGR:
        
            NumChannels = 3;        /* rgb data packed into a 32-bit integer */
            NumCols = theImage.getWidth();    /* # of columns */
            NumRows = theImage.getHeight();    /* # of rows */

            nc = 3;
            if (Type == BufferedImage.TYPE_INT_ARGB) {
                nc = 4;
            }
            chanData = new Object [nc];
            DataOffsets = new int[nc];
            sampleInfo = new int[nc];      /* used to store offsets to bytes
                                              within an int, using 0 for MSB,
                                              3 for LSB */

            OffsetColumn = 4;        /* bytes to next column */

            intRaster = (IntegerComponentRaster) theImage.getRaster();
            OffsetRow = intRaster.getScanlineStride() * 4; /* bytes to next
                                                              row */
            offset = intRaster.getDataOffset(0) * 4;
            dataArray = (Object) intRaster.getDataStorage();
            for (int i = 0; i < 3; i++) {
                chanData[i] = dataArray;
                DataOffsets[i] = offset;
                if (Type == BufferedImage.TYPE_INT_BGR) {
                    sampleInfo[i] = 3 - i;
                } else {
                    sampleInfo[i] = i + 1;
                }
            }
            if (Type == BufferedImage.TYPE_INT_ARGB) {
                chanData[3] = dataArray;
                DataOffsets[3] = offset;
                sampleInfo[3] = 0;
            }

            break;

        case BufferedImage.TYPE_3BYTE_BGR:
        case BufferedImage.TYPE_4BYTE_ABGR:

            NumChannels = 3;        /* rgb data in separate bytes */
            NumCols = theImage.getWidth();    /* # of columns */
            NumRows = theImage.getHeight();    /* # of rows */

            if (Type == BufferedImage.TYPE_3BYTE_BGR) {
                OffsetColumn = 3;        /* bytes to next column */
                nc = 3;
            } else {
                OffsetColumn = 4;        /* bytes to next column */
                nc = 4;
            }
            chanData = new Object [nc];
            DataOffsets = new int[nc];

            byteRaster = (ByteComponentRaster) theImage.getRaster();
            OffsetRow = byteRaster.getScanlineStride(); /* bytes to next row */
            offset = byteRaster.getDataOffset(0);
            dataArray = (Object) byteRaster.getDataStorage();
            for (int i = 0; i < nc; i++) {
                chanData[i] = dataArray;
                DataOffsets[i] = offset - i;
            }

            break;

        case BufferedImage.TYPE_BYTE_GRAY:

            Type = typeComponentUByte;

            NumChannels = 1;
            NumCols = theImage.getWidth();    /* # of columns */
            NumRows = theImage.getHeight();    /* # of rows */

            chanData = new Object [1];             /* just a single address */
            DataOffsets = new int[1];

            OffsetColumn = 1;                      /* bytes to next column */

            byteRaster = (ByteComponentRaster) theImage.getRaster();
            OffsetRow = byteRaster.getScanlineStride(); /* bytes to next row */
            chanData[0] = (Object) byteRaster.getDataStorage();
            DataOffsets[0] = byteRaster.getDataOffset(0);

            break;

        case BufferedImage.TYPE_USHORT_GRAY:

            Type = typeComponentUShort;

            NumChannels = 1;
            NumCols = theImage.getWidth();    /* # of columns */
            NumRows = theImage.getHeight();    /* # of rows */

            chanData = new Object [1];             /* just a single address */
            DataOffsets = new int[1];

            OffsetColumn = 2;                      /* bytes to next column */

            shortRaster = (ShortComponentRaster) theImage.getRaster();
            OffsetRow = shortRaster.getScanlineStride() * 2; /* bytes to next
                                                                row */
            chanData[0] = (Object) shortRaster.getDataStorage();
            DataOffsets[0] = shortRaster.getDataOffset(0) * 2;

            break;

        default:
            // other BufferedImage types handled elsewhere
            throw new IllegalArgumentException(
                "CMMImageLayout - bad image type passed to constructor");
        }
    }


    /* convert a BufferedImage into a common form which is easier for
       the native cmm to use
     */
    public CMMImageLayout(BufferedImage theImage,
                          SinglePixelPackedSampleModel sm,
                          int rPos, int gPos, int bPos, int aPos) {
        // Handle DCM/SPPSM images with arbitrary 8-bit byte-aligned RGB masks
        Type = typeIntRGBPacked;

        NumChannels = 3;        /* rgb data packed into a 32-bit integer */
        NumCols = theImage.getWidth();    /* # of columns */
        NumRows = theImage.getHeight();    /* # of rows */

        int nc = 3;
        if (aPos >= 0) {
            nc = 4;
        }
        chanData = new Object [nc];
        DataOffsets = new int[nc];
        sampleInfo = new int[nc];      /* used to store offsets to bytes
                                          within an int, using 0 for MSB,
                                          3 for LSB - rPos, gPos, bPos, aPos */

        OffsetColumn = 4;        /* bytes to next column */

        OffsetRow = sm.getScanlineStride() * 4; /* bytes to next row */

        Raster ras = theImage.getRaster();
        DataBufferInt dbi = (DataBufferInt) ras.getDataBuffer();
        Object dataArray = (Object) dbi.getData();
        /* Note that Rasters of BufferedImages have minX == minY == 0 */
        int offset = (dbi.getOffset() -
            ras.getSampleModelTranslateY() * sm.getScanlineStride() -
            ras.getSampleModelTranslateX()) * 4;
        for (int i = 0; i < nc; i++) {
            chanData[i] = dataArray;
            DataOffsets[i] = offset;
        }
        sampleInfo[0] = rPos;
        sampleInfo[1] = gPos;
        sampleInfo[2] = bPos;
        if (aPos >= 0) {
            sampleInfo[3] = aPos;
        }
    }


    /* convert a BufferedImage into a common form which is easier for
       the native cmm to use
     */
    public CMMImageLayout(BufferedImage theImage,
                          ComponentSampleModel sm) {
        // Handle byte or ushort component images
        ColorModel cm = theImage.getColorModel();
        int nc = cm.getNumColorComponents();
        boolean hasAlpha = cm.hasAlpha();
        Raster ras = theImage.getRaster();
        int bankIndices[] = sm.getBankIndices();
        int bandOffsets[] = sm.getBandOffsets();
        NumChannels = nc;
        NumCols = theImage.getWidth();    /* # of columns */
        NumRows = theImage.getHeight();    /* # of rows */

        if (hasAlpha) {
            nc += 1;
        }
        chanData = new Object [nc];
        DataOffsets = new int[nc];

        switch (sm.getDataType()) {
            case DataBuffer.TYPE_BYTE:
                {
                    Type = typeComponentUByte;
                    OffsetColumn = sm.getPixelStride();
                    OffsetRow = sm.getScanlineStride();
                    DataBufferByte dbb = (DataBufferByte) ras.getDataBuffer();
                    int dbOffsets[] = dbb.getOffsets();
                    for (int i = 0; i < nc; i++) {
                        chanData[i] = (Object) dbb.getData(bankIndices[i]);
                        /* Note that Rasters of BufferedImages have
                           minX == minY == 0 */
                        DataOffsets[i] = dbOffsets[bankIndices[i]] -
                            ras.getSampleModelTranslateY() *
                            sm.getScanlineStride() -
                            ras.getSampleModelTranslateX() *
                            sm.getPixelStride() +
                            bandOffsets[i];
                    }
                }
                break;

            case DataBuffer.TYPE_USHORT:
                {
                    Type = typeComponentUShort;
                    OffsetColumn = sm.getPixelStride() * 2;
                    OffsetRow = sm.getScanlineStride() * 2;
                    DataBufferUShort dbu =
                        (DataBufferUShort) ras.getDataBuffer();
                    int dbOffsets[] = dbu.getOffsets();
                    for (int i = 0; i < nc; i++) {
                        chanData[i] = (Object) dbu.getData(bankIndices[i]);
                        /* Note that Rasters of BufferedImages have
                           minX == minY == 0 */
                        DataOffsets[i] = (dbOffsets[bankIndices[i]] -
                            ras.getSampleModelTranslateY() *
                            sm.getScanlineStride() -
                            ras.getSampleModelTranslateX() *
                            sm.getPixelStride() +
                            bandOffsets[i]) * 2;
                    }
                }
                break;

            default:
                // other BufferedImage types handled elsewhere
                throw new IllegalArgumentException(
                    "CMMImageLayout - bad image type passed to constructor");
        }
    }


    /* convert a Raster into a common form which is easier for
       the native cmm to use
     */
    public CMMImageLayout(Raster ras, ComponentSampleModel sm) {
        // Handle byte or ushort component images
        int nc = ras.getNumBands();
        int bankIndices[] = sm.getBankIndices();
        int bandOffsets[] = sm.getBandOffsets();
        NumChannels = nc;
        NumCols = ras.getWidth();    /* # of columns */
        NumRows = ras.getHeight();    /* # of rows */

        chanData = new Object [nc];
        DataOffsets = new int[nc];

        switch (sm.getDataType()) {
            case DataBuffer.TYPE_BYTE:
                {
                    Type = typeComponentUByte;
                    OffsetColumn = sm.getPixelStride();
                    OffsetRow = sm.getScanlineStride();
                    DataBufferByte dbb = (DataBufferByte) ras.getDataBuffer();
                    int dbOffsets[] = dbb.getOffsets();
                    for (int i = 0; i < nc; i++) {
                        chanData[i] = (Object) dbb.getData(bankIndices[i]);
                        /* Note that Rasters may have minX, minY != 0 */
                        DataOffsets[i] = dbOffsets[bankIndices[i]] +
                            (ras.getMinY() - ras.getSampleModelTranslateY()) *
                            sm.getScanlineStride() +
                            (ras.getMinX() - ras.getSampleModelTranslateX()) *
                            sm.getPixelStride() +
                            bandOffsets[i];
                    }
                    // Raster assumed to have no alpha channel
                }
                break;

            case DataBuffer.TYPE_USHORT:
                {
                    Type = typeComponentUShort;
                    OffsetColumn = sm.getPixelStride() * 2;
                    OffsetRow = sm.getScanlineStride() * 2;
                    DataBufferUShort dbu =
                        (DataBufferUShort) ras.getDataBuffer();
                    int dbOffsets[] = dbu.getOffsets();
                    for (int i = 0; i < nc; i++) {
                        chanData[i] = (Object) dbu.getData(bankIndices[i]);
                        /* Note that Rasters may have minX, minY != 0 */
                        DataOffsets[i] = (dbOffsets[bankIndices[i]] +
                            (ras.getMinY() - ras.getSampleModelTranslateY()) *
                            sm.getScanlineStride() +
                            (ras.getMinX() - ras.getSampleModelTranslateX()) *
                            sm.getPixelStride() +
                            bandOffsets[i]) * 2;
                    }
                    // Raster assumed to have no alpha channel
                }
                break;

            default:
                // other BufferedImage types handled elsewhere
                throw new IllegalArgumentException(
                    "CMMImageLayout - bad image type passed to constructor");
        }
    }

}
