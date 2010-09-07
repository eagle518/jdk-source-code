/*
 * @(#)imageInitIDs.h	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef IMAGEINITIDS_H
#define IMAGEINITIDS_H

#include "jni.h"

#ifndef IMGEXTERN
# define IMGEXTERN extern
#endif

/* BufferedImage ids */
IMGEXTERN jfieldID g_BImgRasterID;
IMGEXTERN jfieldID g_BImgTypeID;
IMGEXTERN jfieldID g_BImgCMID;
IMGEXTERN jmethodID g_BImgGetRGBMID;
IMGEXTERN jmethodID g_BImgSetRGBMID;

/* Raster ids */
IMGEXTERN jfieldID g_RasterWidthID;
IMGEXTERN jfieldID g_RasterHeightID;
IMGEXTERN jfieldID g_RasterNumBandsID;
IMGEXTERN jfieldID g_RasterBaseRasterID;
IMGEXTERN jfieldID g_RasterMinXID;
IMGEXTERN jfieldID g_RasterMinYID;
IMGEXTERN jfieldID g_RasterBaseOriginXID;
IMGEXTERN jfieldID g_RasterBaseOriginYID;
IMGEXTERN jfieldID g_RasterSampleModelID;
IMGEXTERN jfieldID g_RasterDataBufferID;
IMGEXTERN jfieldID g_RasterNumDataElementsID;
IMGEXTERN jfieldID g_RasterNumBandsID;
IMGEXTERN jmethodID g_RasterGetDataMID;

IMGEXTERN jfieldID g_BCRdataID;
IMGEXTERN jfieldID g_BCRscanstrID;
IMGEXTERN jfieldID g_BCRpixstrID;
IMGEXTERN jfieldID g_BCRbandoffsID;
IMGEXTERN jfieldID g_BCRdataOffsetsID;
IMGEXTERN jfieldID g_BCRtypeID;
IMGEXTERN jfieldID g_BPRdataID;
IMGEXTERN jfieldID g_BPRscanstrID;
IMGEXTERN jfieldID g_BPRpixstrID;
IMGEXTERN jfieldID g_BPRtypeID;
IMGEXTERN jfieldID g_BPRdataBitOffsetID;
IMGEXTERN jfieldID g_SCRdataID;
IMGEXTERN jfieldID g_SCRscanstrID;
IMGEXTERN jfieldID g_SCRpixstrID;
IMGEXTERN jfieldID g_SCRbandoffsID;
IMGEXTERN jfieldID g_SCRdataOffsetsID;
IMGEXTERN jfieldID g_SCRtypeID;
IMGEXTERN jfieldID g_ICRdataID;
IMGEXTERN jfieldID g_ICRscanstrID;
IMGEXTERN jfieldID g_ICRpixstrID;
IMGEXTERN jfieldID g_ICRbandoffsID;
IMGEXTERN jfieldID g_ICRdataOffsetsID;
IMGEXTERN jfieldID g_ICRtypeID;
IMGEXTERN jmethodID g_ICRputDataMID;

/* Color Model ids */
IMGEXTERN jfieldID g_CMpDataID;
IMGEXTERN jfieldID g_CMnBitsID;
IMGEXTERN jfieldID g_CMcspaceID;
IMGEXTERN jfieldID g_CMnumComponentsID;
IMGEXTERN jfieldID g_CMsuppAlphaID;
IMGEXTERN jfieldID g_CMisAlphaPreID;
IMGEXTERN jfieldID g_CMtransparencyID;
IMGEXTERN jmethodID g_CMgetRGBMID;
IMGEXTERN jfieldID g_CMcsTypeID;
IMGEXTERN jfieldID g_CMis_sRGBID;
IMGEXTERN jmethodID g_CMgetRGBdefaultMID;

IMGEXTERN jfieldID g_ICMtransIdxID;
IMGEXTERN jfieldID g_ICMmapSizeID;
IMGEXTERN jfieldID g_ICMrgbID;

/* Sample Model ids */
IMGEXTERN jfieldID g_SMWidthID;
IMGEXTERN jfieldID g_SMHeightID;
IMGEXTERN jmethodID g_SMGetPixelsMID;
IMGEXTERN jmethodID g_SMSetPixelsMID;

/* Single Pixel Packed Sample Model ids */
IMGEXTERN jfieldID g_SPPSMmaskArrID;
IMGEXTERN jfieldID g_SPPSMmaskOffID;
IMGEXTERN jfieldID g_SPPSMnBitsID;
IMGEXTERN jfieldID g_SPPSMmaxBitID;

/* Component Sample Model ids */
IMGEXTERN jfieldID g_CSMPixStrideID;
IMGEXTERN jfieldID g_CSMScanStrideID;
IMGEXTERN jfieldID g_CSMBandOffsetsID;

/* Kernel ids */
IMGEXTERN jfieldID g_KernelWidthID;
IMGEXTERN jfieldID g_KernelHeightID;
IMGEXTERN jfieldID g_KernelXOriginID;
IMGEXTERN jfieldID g_KernelYOriginD;
IMGEXTERN jfieldID g_KernelDataID;

/* DataBufferInt ids */
IMGEXTERN jfieldID g_DataBufferIntPdataID;

#endif /* IMAGEINITIDS_H */
